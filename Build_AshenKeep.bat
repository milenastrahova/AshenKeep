@echo off
set "UE_BUILD=C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat"
set "PROJECT=C:\Users\user\Documents\Unreal Projects\AshenKeep\AshenKeep.uproject"

call "%UE_BUILD%" AshenKeepEditor Win64 Development -Project="%PROJECT%" -WaitMutex -NoHotReloadFromIDE

echo.
if errorlevel 1 (
    echo BUILD FAILED
) else (
    echo BUILD SUCCEEDED
)
pause
