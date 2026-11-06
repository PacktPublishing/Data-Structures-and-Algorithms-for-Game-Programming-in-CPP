[CmdletBinding()]
param(
    [ValidateRange(1, 86400)]
    [int]$RunSeconds = 120,
    [string]$DeviceSerial,
    [string[]]$Samples,
    [string]$StartAt,
    [string]$ResultsDirectory = (Join-Path $PSScriptRoot "smoke-results")
)

$ErrorActionPreference = "Stop"
$packageName = "com.knight.cookbook"
$activityName = "android.app.NativeActivity"
$repoRoot = Split-Path $PSScriptRoot -Parent
$gameCmake = Join-Path $repoRoot "Knight\CMakeLists.txt"
$gradleWrapper = Join-Path $PSScriptRoot "gradlew.bat"
$apkPath = Join-Path $PSScriptRoot "app\build\outputs\apk\debug\app-debug.apk"

function Get-KnightGames {
    $text = Get-Content -LiteralPath $gameCmake -Raw
    $match = [regex]::Match($text, 'set\s*\(KNIGHT_GAME_DIRS(?<games>.*?)\)', 'Singleline')
    if (-not $match.Success) {
        throw "Could not find KNIGHT_GAME_DIRS in $gameCmake"
    }

    return @([regex]::Matches($match.Groups['games'].Value, '[A-Za-z][A-Za-z0-9]*') |
        ForEach-Object { $_.Value })
}

function Find-Adb {
    $command = Get-Command adb -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }

    foreach ($sdkVariable in @('ANDROID_HOME', 'ANDROID_SDK_ROOT')) {
        $sdk = [Environment]::GetEnvironmentVariable($sdkVariable)
        if ($sdk) {
            $candidate = Join-Path $sdk 'platform-tools\adb.exe'
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }

    $localProperties = Join-Path $PSScriptRoot 'local.properties'
    if (Test-Path -LiteralPath $localProperties) {
        $sdkLine = Get-Content -LiteralPath $localProperties |
            Where-Object { $_ -match '^sdk\.dir=' } |
            Select-Object -First 1
        if ($sdkLine) {
            $sdk = ($sdkLine -replace '^sdk\.dir=', '') -replace '\\:', ':' -replace '\\\\', '\'
            $candidate = Join-Path $sdk 'platform-tools\adb.exe'
            if (Test-Path -LiteralPath $candidate) { return $candidate }
        }
    }

    throw 'adb.exe was not found. Add Android SDK platform-tools to PATH or configure android/local.properties.'
}

function Invoke-Adb {
    $allArguments = @()
    if ($DeviceSerial) { $allArguments += @('-s', $DeviceSerial) }
    $allArguments += $args

    # Windows PowerShell surfaces native stderr as NativeCommandError records.
    # adb writes benign progress (notably `adb pull`) to stderr even on success,
    # so do not let the script's stop-on-error policy turn that into an exception.
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $script:adb @allArguments
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
}

$allGames = Get-KnightGames
if ($Samples) {
    $unknown = @($Samples | Where-Object { $_ -notin $allGames })
    if ($unknown.Count -gt 0) { throw "Unknown sample(s): $($unknown -join ', ')" }
    $games = @($Samples)
} else {
    $games = @($allGames)
}

if ($StartAt) {
    $startIndex = [array]::IndexOf($games, $StartAt)
    if ($startIndex -lt 0) { throw "StartAt sample '$StartAt' is not in the selected sample list." }
    $games = @($games[$startIndex..($games.Count - 1)])
}

$script:adb = Find-Adb
$deviceLines = @(& $script:adb devices | Select-Object -Skip 1 | Where-Object { $_ -match '\sdevice\s*$' })
if (-not $DeviceSerial -and $deviceLines.Count -ne 1) {
    throw "Expected exactly one authorized device, found $($deviceLines.Count). Use -DeviceSerial when multiple devices are connected."
}
if ($DeviceSerial -and -not (& $script:adb -s $DeviceSerial get-state 2>$null | Select-String '^device$')) {
    throw "Device '$DeviceSerial' is not connected and authorized."
}

$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$runDirectory = Join-Path $ResultsDirectory $timestamp
New-Item -ItemType Directory -Path $runDirectory -Force | Out-Null
$results = [System.Collections.Generic.List[object]]::new()
$failurePattern = 'FATAL EXCEPTION|Fatal signal|ANR in com\.knight\.cookbook|am_crash|am_anr|DEBUG\s*:.*backtrace'

Write-Host "Testing $($games.Count) samples for $RunSeconds seconds each."
Write-Host "Results: $runDirectory"

try {
    Invoke-Adb shell svc power stayon true | Out-Null

    foreach ($game in $games) {
        $sampleDirectory = Join-Path $runDirectory $game
        New-Item -ItemType Directory -Path $sampleDirectory -Force | Out-Null
        $status = 'Pass'
        $detail = ''
        Write-Host "`n=== $game ===" -ForegroundColor Cyan

        & $gradleWrapper ':app:assembleDebug' "-PknightGame=$game" '--no-parallel' 2>&1 |
            Tee-Object -FilePath (Join-Path $sampleDirectory 'build.log')
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $apkPath)) {
            $status = 'BuildFailed'
            $detail = "Gradle exit code $LASTEXITCODE"
        }

        if ($status -eq 'Pass') {
            Invoke-Adb install -r -t $apkPath 2>&1 |
                Tee-Object -FilePath (Join-Path $sampleDirectory 'install.log')
            if ($LASTEXITCODE -ne 0) {
                $status = 'InstallFailed'
                $detail = "adb install exit code $LASTEXITCODE"
            }
        }

        if ($status -eq 'Pass') {
            Invoke-Adb logcat -c
            Invoke-Adb shell am force-stop $packageName | Out-Null
            Invoke-Adb shell am start -W -n "$packageName/$activityName" 2>&1 |
                Tee-Object -FilePath (Join-Path $sampleDirectory 'launch.log')
            if ($LASTEXITCODE -ne 0) {
                $status = 'LaunchFailed'
                $detail = "am start exit code $LASTEXITCODE"
            }
        }

        if ($status -eq 'Pass') {
            $deadline = (Get-Date).AddSeconds($RunSeconds)
            Start-Sleep -Seconds 2
            while ((Get-Date) -lt $deadline) {
                $appProcessId = (Invoke-Adb shell pidof $packageName 2>$null | Out-String).Trim()
                if (-not $appProcessId) {
                    $status = 'ProcessExited'
                    $detail = 'Application process exited before the observation period ended.'
                    break
                }
                Start-Sleep -Seconds ([Math]::Min(2, [Math]::Max(1, [int]($deadline - (Get-Date)).TotalSeconds)))
            }
        }

        if ($status -notin @('BuildFailed', 'InstallFailed')) {
            Invoke-Adb logcat -d -v threadtime 2>&1 |
                Set-Content -LiteralPath (Join-Path $sampleDirectory 'logcat.txt') -Encoding UTF8
            Invoke-Adb shell screencap -p /sdcard/knight-smoke.png | Out-Null
            Invoke-Adb pull /sdcard/knight-smoke.png (Join-Path $sampleDirectory 'screenshot.png') 2>&1 |
                Set-Content -LiteralPath (Join-Path $sampleDirectory 'screenshot-pull.log')
            Invoke-Adb shell rm /sdcard/knight-smoke.png | Out-Null

            $crashLines = @(Select-String -LiteralPath (Join-Path $sampleDirectory 'logcat.txt') -Pattern $failurePattern)
            if ($status -eq 'Pass' -and $crashLines.Count -gt 0) {
                $status = 'RuntimeFailure'
                $detail = $crashLines[0].Line.Trim()
            }
            Invoke-Adb shell am force-stop $packageName | Out-Null
        }

        $results.Add([pscustomobject]@{
            Sample = $game
            Result = $status
            Detail = $detail
        })
        $color = if ($status -eq 'Pass') { 'Green' } else { 'Red' }
        Write-Host "$game : $status $detail" -ForegroundColor $color
    }
}
finally {
    try { Invoke-Adb shell am force-stop $packageName 2>$null | Out-Null } catch {}
    $results | Export-Csv -LiteralPath (Join-Path $runDirectory 'summary.csv') -NoTypeInformation -Encoding UTF8
    $results | Format-Table -AutoSize | Out-String |
        Set-Content -LiteralPath (Join-Path $runDirectory 'summary.txt') -Encoding UTF8
}

$results | Format-Table -AutoSize
$failedCount = @($results | Where-Object Result -ne 'Pass').Count
Write-Host "Summary: $($results.Count - $failedCount) passed, $failedCount failed."
if ($failedCount -gt 0) { exit 1 }

