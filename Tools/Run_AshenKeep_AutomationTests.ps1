$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$UProjectPath = Join-Path $ProjectRoot "AshenKeep.uproject"
$Desktop = [Environment]::GetFolderPath("Desktop")
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$LogPath = Join-Path $Desktop "AshenKeep_AutomationTests_$Timestamp.log"

$EngineCandidates = @(
    "C:\Program Files\Epic Games\UE_5.8"
)

$EngineCandidates += Get-ChildItem `
    -LiteralPath "C:\Program Files\Epic Games" `
    -Directory `
    -Filter "UE_*" `
    -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending |
    ForEach-Object { $_.FullName }

$EngineRoot = $EngineCandidates |
    Where-Object {
        Test-Path -LiteralPath (
            Join-Path $_ "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        )
    } |
    Select-Object -First 1

if (-not $EngineRoot) {
    throw "Unreal Engine installation was not found."
}

$EditorCmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

Write-Host "Running AshenKeep.* automation tests..." -ForegroundColor Cyan
Write-Host "Log: $LogPath" -ForegroundColor White

& $EditorCmd `
    "$UProjectPath" `
    -unattended `
    -nop4 `
    -NullRHI `
    -NoSound `
    -nosplash `
    "-ExecCmds=Automation RunTests AshenKeep; Quit" `
    "-TestExit=Automation Test Queue Empty" `
    "-log=$LogPath"

$ExitCode = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $LogPath)) {
    throw "Automation log was not created."
}

$LogText = Get-Content -LiteralPath $LogPath -Raw

if ($ExitCode -ne 0 -or
    $LogText -match "Result=\{Fail\}|Automation Test Failed|Test Failed") {
    Start-Process notepad.exe $LogPath
    throw "One or more Ashen Keep automation tests failed."
}

Write-Host "ALL ASHEN KEEP AUTOMATION TESTS PASSED." -ForegroundColor Green
Start-Process notepad.exe $LogPath