#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
GAME_CMAKE="$REPO_ROOT/Knight/CMakeLists.txt"
GRADLEW="$SCRIPT_DIR/gradlew"
APK_PATH="$SCRIPT_DIR/app/build/outputs/apk/debug/app-debug.apk"
PACKAGE_NAME="com.knight.cookbook"
ACTIVITY_NAME=".KnightActivity"
RUN_SECONDS=120
DEVICE_SERIAL=""
START_AT=""
RESULTS_ROOT="$SCRIPT_DIR/smoke-results"
SELECTED_SAMPLES=()

usage() {
    cat <<'EOF'
Usage: ./smoke-test-all.sh [options] [Sample ...]

Options:
  --seconds N       Run each sample for N seconds (default: 120)
  --device SERIAL   Select a device when more than one is connected
  --start-at NAME   Skip samples before NAME
  --results DIR     Root directory for results
  -h, --help        Show this help

With no Sample arguments, all entries in KNIGHT_SAMPLE_DIRS are tested.
EOF
}

while (($#)); do
    case "$1" in
        --seconds) RUN_SECONDS="$2"; shift 2 ;;
        --device) DEVICE_SERIAL="$2"; shift 2 ;;
        --start-at) START_AT="$2"; shift 2 ;;
        --results) RESULTS_ROOT="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        --*) echo "Unknown option: $1" >&2; usage; exit 2 ;;
        *) SELECTED_SAMPLES+=("$1"); shift ;;
    esac
done

if ! [[ "$RUN_SECONDS" =~ ^[1-9][0-9]*$ ]]; then
    echo "--seconds must be a positive integer." >&2
    exit 2
fi

ALL_GAMES=()
while IFS= read -r game; do
    ALL_GAMES+=("$game")
done < <(awk '
        /set[[:space:]]*\(KNIGHT_SAMPLE_DIRS/ { capture=1; sub(/^.*KNIGHT_SAMPLE_DIRS/, ""); }
        capture {
            line=$0
            sub(/#.*/, "", line)
            done=(line ~ /\)/)
            gsub(/[()]/, " ", line)
            count=split(line, words, /[[:space:]]+/)
            for (i=1; i<=count; i++) if (words[i] ~ /^(Bonus|Demo)[A-Za-z0-9]+$/) print words[i]
            if (done) exit
        }
    ' "$GAME_CMAKE")

if ((${#ALL_GAMES[@]} == 0)); then
    echo "Could not read KNIGHT_SAMPLE_DIRS from $GAME_CMAKE" >&2
    exit 2
fi

contains_game() {
    local wanted="$1" game
    for game in "${ALL_GAMES[@]}"; do [[ "$game" == "$wanted" ]] && return 0; done
    return 1
}

if ((${#SELECTED_SAMPLES[@]})); then
    GAMES=("${SELECTED_SAMPLES[@]}")
    for game in "${GAMES[@]}"; do
        contains_game "$game" || { echo "Unknown sample: $game" >&2; exit 2; }
    done
else
    GAMES=("${ALL_GAMES[@]}")
fi

if [[ -n "$START_AT" ]]; then
    found=0
    FILTERED_GAMES=()
    for game in "${GAMES[@]}"; do
        [[ "$game" == "$START_AT" ]] && found=1
        ((found)) && FILTERED_GAMES+=("$game")
    done
    ((found)) || { echo "--start-at sample '$START_AT' is not selected." >&2; exit 2; }
    GAMES=("${FILTERED_GAMES[@]}")
fi

if command -v adb >/dev/null 2>&1; then
    ADB="$(command -v adb)"
elif [[ -n "${ANDROID_HOME:-}" && -x "${ANDROID_HOME:-}/platform-tools/adb" ]]; then
    ADB="${ANDROID_HOME:-}/platform-tools/adb"
elif [[ -n "${ANDROID_SDK_ROOT:-}" && -x "${ANDROID_SDK_ROOT:-}/platform-tools/adb" ]]; then
    ADB="${ANDROID_SDK_ROOT:-}/platform-tools/adb"
else
    SDK_DIR="$(sed -n 's/^sdk\.dir=//p' "$SCRIPT_DIR/local.properties" 2>/dev/null | head -n 1)"
    [[ -n "$SDK_DIR" && -x "$SDK_DIR/platform-tools/adb" ]] || {
        echo "adb was not found. Add platform-tools to PATH or configure local.properties." >&2
        exit 2
    }
    ADB="$SDK_DIR/platform-tools/adb"
fi

adb_cmd() {
    if [[ -n "$DEVICE_SERIAL" ]]; then "$ADB" -s "$DEVICE_SERIAL" "$@"; else "$ADB" "$@"; fi
}

if [[ -n "$DEVICE_SERIAL" ]]; then
    [[ "$(adb_cmd get-state 2>/dev/null)" == "device" ]] || {
        echo "Device '$DEVICE_SERIAL' is not connected and authorized." >&2; exit 2;
    }
else
    DEVICE_COUNT="$($ADB devices | awk '$2 == "device" { count++ } END { print count+0 }')"
    [[ "$DEVICE_COUNT" == "1" ]] || {
        echo "Expected exactly one authorized device, found $DEVICE_COUNT. Use --device when needed." >&2; exit 2;
    }
fi

RUN_DIR="$RESULTS_ROOT/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN_DIR"
SUMMARY="$RUN_DIR/summary.csv"
printf 'Sample,Result,Detail\n' > "$SUMMARY"
FAILURES=0
PASSES=0

csv_escape() { local value=${1//\"/\"\"}; printf '"%s"' "$value"; }
record_result() {
    local game="$1" status="$2" detail="$3"
    { csv_escape "$game"; printf ','; csv_escape "$status"; printf ','; csv_escape "$detail"; printf '\n'; } >> "$SUMMARY"
    if [[ "$status" == "Pass" ]]; then ((PASSES+=1)); else ((FAILURES+=1)); fi
    printf '%-28s %-16s %s\n' "$game" "$status" "$detail"
}

cleanup() { adb_cmd shell am force-stop "$PACKAGE_NAME" >/dev/null 2>&1 || true; }
trap cleanup EXIT INT TERM
adb_cmd shell svc power stayon true >/dev/null 2>&1 || true

echo "Testing ${#GAMES[@]} samples for $RUN_SECONDS seconds each."
echo "Results: $RUN_DIR"

for game in "${GAMES[@]}"; do
    SAMPLE_DIR="$RUN_DIR/$game"
    mkdir -p "$SAMPLE_DIR"
    status="Pass"
    detail=""
    echo
    echo "=== $game ==="

    if ! "$GRADLEW" :app:assembleDebug "-PknightGame=$game" --no-parallel 2>&1 | tee "$SAMPLE_DIR/build.log"; then
        status="BuildFailed"; detail="Gradle build failed"
    elif [[ ! -f "$APK_PATH" ]]; then
        status="BuildFailed"; detail="APK was not produced at $APK_PATH"
    fi

    if [[ "$status" == "Pass" ]]; then
        if ! adb_cmd install -r -t "$APK_PATH" 2>&1 | tee "$SAMPLE_DIR/install.log"; then
            status="InstallFailed"; detail="adb install failed"
        fi
    fi

    if [[ "$status" == "Pass" ]]; then
        adb_cmd logcat -c
        adb_cmd shell am force-stop "$PACKAGE_NAME" >/dev/null
        if ! adb_cmd shell am start -W -n "$PACKAGE_NAME/$ACTIVITY_NAME" 2>&1 | tee "$SAMPLE_DIR/launch.log"; then
            status="LaunchFailed"; detail="am start failed"
        fi
    fi

    if [[ "$status" == "Pass" ]]; then
        deadline=$((SECONDS + RUN_SECONDS))
        sleep 2
        while ((SECONDS < deadline)); do
            if [[ -z "$(adb_cmd shell pidof "$PACKAGE_NAME" 2>/dev/null | tr -d '\r')" ]]; then
                status="ProcessExited"; detail="Application process exited before the observation period ended."
                break
            fi
            sleep 2
        done
    fi

    if [[ "$status" != "BuildFailed" && "$status" != "InstallFailed" ]]; then
        adb_cmd logcat -d -v threadtime > "$SAMPLE_DIR/logcat.txt" 2>&1 || true
        adb_cmd shell screencap -p /sdcard/knight-smoke.png >/dev/null 2>&1 || true
        adb_cmd pull /sdcard/knight-smoke.png "$SAMPLE_DIR/screenshot.png" > "$SAMPLE_DIR/screenshot-pull.log" 2>&1 || true
        adb_cmd shell rm /sdcard/knight-smoke.png >/dev/null 2>&1 || true
        if [[ "$status" == "Pass" ]] && grep -E -m1 'FATAL EXCEPTION|Fatal signal|ANR in com\.knight\.cookbook|am_crash|am_anr|DEBUG[[:space:]]*:.*backtrace' "$SAMPLE_DIR/logcat.txt" > "$SAMPLE_DIR/failure-match.txt"; then
            status="RuntimeFailure"
            detail="$(head -n 1 "$SAMPLE_DIR/failure-match.txt" | tr -d '\r')"
        fi
        adb_cmd shell am force-stop "$PACKAGE_NAME" >/dev/null 2>&1 || true
    fi

    record_result "$game" "$status" "$detail"
done

echo
echo "Summary: $PASSES passed, $FAILURES failed."
echo "CSV report: $SUMMARY"
((FAILURES == 0))

