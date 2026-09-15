@echo off
setlocal enabledelayedexpansion

REM ==============================================================================
REM TimeEngine Headless Stress & Benchmark Test Runner (Windows)
REM Runs stress tests against the current built binary without compiling.
REM ==============================================================================

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%..\..\"
set "EXE_DEBUG=%ROOT_DIR%Artifacts\Bin\Debug-windows-x86_64\TimeEditor\TimeEditor.exe"
set "EXE_RELEASE=%ROOT_DIR%Artifacts\Bin\Release-windows-x86_64\TimeEditor\TimeEditor.exe"

set "TARGET_EXE="
if exist "%EXE_DEBUG%" (
    set "TARGET_EXE=%EXE_DEBUG%"
) else if exist "%EXE_RELEASE%" (
    set "TARGET_EXE=%EXE_RELEASE%"
)

if "%TARGET_EXE%"=="" (
    echo [ERROR] TimeEngine executable not found in Artifacts\Bin\Debug-windows-x86_64 or Artifacts\Bin\Release-windows-x86_64.
    echo Please build the project first using Scripts\Windows\MSVC\BuildDebug.bat
    exit /b 1
)

echo [TimeEngine] Executing headless stress benchmarks with: %TARGET_EXE%
"%TARGET_EXE%" --headless --test --stress %*
exit /b %ERRORLEVEL%
