[CmdletBinding()]
param(
    [ValidateRange(1, 86400)]
    [int]$RunSeconds = 120,
    [ValidateSet('web-debug', 'web-release')]
    [string]$Preset = 'web-debug',
    [string[]]$Samples,
    [string]$StartAt,
    [string]$Browser,
    [string]$ResultsDirectory = (Join-Path $PSScriptRoot 'smoke-results')
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent

function Invoke-LoggedCommand {
    param(
        [Parameter(Mandatory, Position = 0)]
        [string[]]$Command,
        [Parameter(Mandatory, Position = 1)]
        [string]$LogPath
    )

    $executable = $Command[0]
    $arguments = @($Command | Select-Object -Skip 1)
    $previousErrorActionPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $executable @arguments 2>&1 | Tee-Object -FilePath $LogPath | Out-Host
        return $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
    }
}
$buildDirectory = Join-Path $repoRoot "out\build\$Preset"
$gameList = Join-Path $buildDirectory 'knight-registered-games.txt'

foreach ($tool in @('cmake', 'emrun')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool was not found. Activate emsdk and ensure CMake is on PATH."
    }
}

$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$runDirectory = Join-Path $ResultsDirectory $timestamp
New-Item -ItemType Directory -Path $runDirectory -Force | Out-Null

$configureLog = Join-Path $runDirectory 'configure.log'
$configureExit = Invoke-LoggedCommand @('cmake', '--preset', $Preset) $configureLog
if ($configureExit -ne 0 -or -not (Test-Path -LiteralPath $gameList)) {
    throw "Initial CMake configure failed; see $configureLog"
}

$allGames = @(Get-Content -LiteralPath $gameList | Where-Object { $_.Trim() })
if ($Samples) {
    $unknown = @($Samples | Where-Object { $_ -notin $allGames })
    if ($unknown) { throw "Unknown sample(s): $($unknown -join ', ')" }
    $games = @($Samples)
} else { $games = @($allGames) }

if ($StartAt) {
    $startIndex = [array]::IndexOf($games, $StartAt)
    if ($startIndex -lt 0) { throw "StartAt game '$StartAt' is not selected." }
    $games = @($games[$startIndex..($games.Count - 1)])
}

$results = [System.Collections.Generic.List[object]]::new()
Write-Host "Testing $($games.Count) web games for $RunSeconds seconds each."
Write-Host "Results: $runDirectory"

foreach ($game in $games) {
    $sampleDirectory = Join-Path $runDirectory $game
    New-Item -ItemType Directory -Path $sampleDirectory -Force | Out-Null
    $status = 'Pass'
    $detail = ''
    Write-Host "`n=== $game ===" -ForegroundColor Cyan

    $exitCode = Invoke-LoggedCommand @('cmake', '--preset', $Preset, "-DKNIGHT_GAME=$game") (Join-Path $sampleDirectory 'configure.log')
    if ($exitCode -ne 0) { $status = 'ConfigureFailed'; $detail = "CMake exit code $exitCode" }

    if ($status -eq 'Pass') {
        $exitCode = Invoke-LoggedCommand @('cmake', '--build', '--preset', $Preset, '--target', $game) (Join-Path $sampleDirectory 'build.log')
        if ($exitCode -ne 0) { $status = 'BuildFailed'; $detail = "CMake exit code $exitCode" }
    }

    $htmlPath = Join-Path $buildDirectory "bin\$game.html"
    if ($status -eq 'Pass' -and -not (Test-Path -LiteralPath $htmlPath)) {
        $status = 'BuildFailed'; $detail = "HTML was not produced at $htmlPath"
    }

    $server = $null
    if ($status -eq 'Pass') {
        $arguments = @()
        if ($Browser) { $arguments += @('--browser', $Browser) }
        $arguments += $htmlPath
        $server = Start-Process -FilePath (Get-Command emrun).Source -ArgumentList $arguments `
            -RedirectStandardOutput (Join-Path $sampleDirectory 'emrun.stdout.log') `
            -RedirectStandardError (Join-Path $sampleDirectory 'emrun.stderr.log') `
            -PassThru
        $deadline = (Get-Date).AddSeconds($RunSeconds)
        Start-Sleep -Seconds ([Math]::Min(2, $RunSeconds))
        while ((Get-Date) -lt $deadline -and -not $server.HasExited) {
            Start-Sleep -Seconds ([Math]::Min(2, [Math]::Max(1, [int]($deadline - (Get-Date)).TotalSeconds)))
            $server.Refresh()
        }
        if ($server.HasExited) {
            $status = 'RuntimeFailure'; $detail = "emrun exited early with code $($server.ExitCode)"
        } else {
            Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
            $server.WaitForExit()
        }
    }

    $results.Add([pscustomobject]@{ Game = $game; Result = $status; Detail = $detail })
    $color = if ($status -eq 'Pass') { 'Green' } else { 'Red' }
    Write-Host "$game : $status $detail" -ForegroundColor $color
}

$results | Export-Csv -LiteralPath (Join-Path $runDirectory 'summary.csv') -NoTypeInformation -Encoding UTF8
$results | Format-Table -AutoSize | Out-String | Set-Content -LiteralPath (Join-Path $runDirectory 'summary.txt') -Encoding UTF8
$results | Format-Table -AutoSize
$failedCount = @($results | Where-Object Result -ne 'Pass').Count
Write-Host "Summary: $($results.Count - $failedCount) passed, $failedCount failed."
if ($failedCount -gt 0) { exit 1 }
