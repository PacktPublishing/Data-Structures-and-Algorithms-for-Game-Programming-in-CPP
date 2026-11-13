#!/usr/bin/env bash
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RUN_SECONDS=120
PRESET="web-debug"
START_AT=""
BROWSER=""
RESULTS_ROOT="$SCRIPT_DIR/smoke-results"
SELECTED_GAMES=()

usage() {
    cat <<'EOF'
Usage: ./web/smoke-test-all.sh [options] [Game ...]

Options:
  --seconds N       Run each game for N seconds (default: 120)
  --preset NAME     web-debug or web-release (default: web-debug)
  --browser NAME    Browser passed to emrun
  --start-at NAME   Skip games before NAME
  --results DIR     Root directory for results
  -h, --help        Show this help

With no Game arguments, every CMake-registered sample and user game is tested.
EOF
}

while (($#)); do
    case "$1" in
        --seconds) RUN_SECONDS="$2"; shift 2 ;;
        --preset) PRESET="$2"; shift 2 ;;
        --browser) BROWSER="$2"; shift 2 ;;
        --start-at) START_AT="$2"; shift 2 ;;
        --results) RESULTS_ROOT="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        --*) echo "Unknown option: $1" >&2; usage; exit 2 ;;
        *) SELECTED_GAMES+=("$1"); shift ;;
    esac
done

[[ "$RUN_SECONDS" =~ ^[1-9][0-9]*$ ]] || { echo "--seconds must be a positive integer." >&2; exit 2; }
[[ "$PRESET" == web-debug || "$PRESET" == web-release ]] || { echo "--preset must be web-debug or web-release." >&2; exit 2; }
command -v cmake >/dev/null || { echo "cmake was not found." >&2; exit 2; }
command -v emrun >/dev/null || { echo "emrun was not found. Activate emsdk first." >&2; exit 2; }

BUILD_DIR="$REPO_ROOT/out/build/$PRESET"
GAME_LIST="$BUILD_DIR/knight-registered-games.txt"
RUN_DIR="$RESULTS_ROOT/$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN_DIR"
cmake --preset "$PRESET" 2>&1 | tee "$RUN_DIR/configure.log"
configure_status=${PIPESTATUS[0]}
((configure_status == 0)) && [[ -f "$GAME_LIST" ]] || { echo "Initial CMake configure failed." >&2; exit 1; }
mapfile -t ALL_GAMES < "$GAME_LIST"

contains_game() { local wanted="$1" game; for game in "${ALL_GAMES[@]}"; do [[ "$game" == "$wanted" ]] && return 0; done; return 1; }
if ((${#SELECTED_GAMES[@]})); then
    GAMES=("${SELECTED_GAMES[@]}")
    for game in "${GAMES[@]}"; do contains_game "$game" || { echo "Unknown game: $game" >&2; exit 2; }; done
else GAMES=("${ALL_GAMES[@]}"); fi

if [[ -n "$START_AT" ]]; then
    found=0; FILTERED_GAMES=()
    for game in "${GAMES[@]}"; do [[ "$game" == "$START_AT" ]] && found=1; ((found)) && FILTERED_GAMES+=("$game"); done
    ((found)) || { echo "--start-at game '$START_AT' is not selected." >&2; exit 2; }
    GAMES=("${FILTERED_GAMES[@]}")
fi

SUMMARY="$RUN_DIR/summary.csv"
printf 'Game,Result,Detail\n' > "$SUMMARY"
PASSES=0; FAILURES=0; EMRUN_PID=""
csv_escape() { local value=${1//\"/\"\"}; printf '"%s"' "$value"; }
record_result() {
    { csv_escape "$1"; printf ','; csv_escape "$2"; printf ','; csv_escape "$3"; printf '\n'; } >> "$SUMMARY"
    if [[ "$2" == Pass ]]; then ((PASSES+=1)); else ((FAILURES+=1)); fi
    printf '%-28s %-16s %s\n' "$1" "$2" "$3"
}
cleanup() { [[ -n "$EMRUN_PID" ]] && kill "$EMRUN_PID" >/dev/null 2>&1 || true; }
trap cleanup EXIT INT TERM

echo "Testing ${#GAMES[@]} web games for $RUN_SECONDS seconds each."
echo "Results: $RUN_DIR"
for game in "${GAMES[@]}"; do
    SAMPLE_DIR="$RUN_DIR/$game"; mkdir -p "$SAMPLE_DIR"
    status=Pass; detail=""; echo; echo "=== $game ==="
    if ! cmake --preset "$PRESET" "-DKNIGHT_GAME=$game" 2>&1 | tee "$SAMPLE_DIR/configure.log"; then
        status=ConfigureFailed; detail="CMake configure failed"
    elif ! cmake --build --preset "$PRESET" --target "$game" 2>&1 | tee "$SAMPLE_DIR/build.log"; then
        status=BuildFailed; detail="CMake build failed"
    fi
    HTML_PATH="$BUILD_DIR/bin/$game.html"
    if [[ "$status" == Pass && ! -f "$HTML_PATH" ]]; then status=BuildFailed; detail="HTML was not produced at $HTML_PATH"; fi

    if [[ "$status" == Pass ]]; then
        emrun_args=(); [[ -n "$BROWSER" ]] && emrun_args+=(--browser "$BROWSER"); emrun_args+=("$HTML_PATH")
        emrun "${emrun_args[@]}" >"$SAMPLE_DIR/emrun.stdout.log" 2>"$SAMPLE_DIR/emrun.stderr.log" &
        EMRUN_PID=$!
        deadline=$((SECONDS + RUN_SECONDS)); sleep "$((RUN_SECONDS < 2 ? RUN_SECONDS : 2))"
        while ((SECONDS < deadline)) && kill -0 "$EMRUN_PID" 2>/dev/null; do sleep 2; done
        if ! kill -0 "$EMRUN_PID" 2>/dev/null; then
            wait "$EMRUN_PID"; code=$?; status=RuntimeFailure; detail="emrun exited early with code $code"
        else
            kill "$EMRUN_PID" >/dev/null 2>&1 || true; wait "$EMRUN_PID" 2>/dev/null || true
        fi
        EMRUN_PID=""
    fi
    record_result "$game" "$status" "$detail"
done

echo; echo "Summary: $PASSES passed, $FAILURES failed."; echo "CSV report: $SUMMARY"
((FAILURES == 0))
