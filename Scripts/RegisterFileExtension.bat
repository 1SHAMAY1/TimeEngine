@echo off
pushd %~dp0\..

:: Search for Sandbox.exe in common build locations
set "ENGINE_EXE=Bin\Debug-windows-x86_64\Sandbox\Sandbox.exe"
if not exist "%ENGINE_EXE%" set "ENGINE_EXE=Bin\Release-windows-x86_64\Sandbox\Sandbox.exe"
if not exist "%ENGINE_EXE%" set "ENGINE_EXE=Bin\Debug-windows-x86_64\Engine\Sandbox.exe"
if not exist "%ENGINE_EXE%" set "ENGINE_EXE=Bin\Release-windows-x86_64\Engine\Sandbox.exe"

if exist "%ENGINE_EXE%" (
    echo [TimeEngine] Registering .teproj file extension...
    echo [TimeEngine] Found executable: %ENGINE_EXE%
    start "" "%ENGINE_EXE%" --register
    echo [TimeEngine] Registration process started.
) else (
    echo [TimeEngine] ERROR: Sandbox.exe not found!
    echo [TimeEngine] Please build the Sandbox project first before running this script.
    pause
)

popd
