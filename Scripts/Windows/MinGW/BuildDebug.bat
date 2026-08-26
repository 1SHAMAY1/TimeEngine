@echo off
title Building TimeEngine (MinGW Debug)
echo [=== Building TimeEngine (MinGW Debug) ===]

:: Go to root (3 directories back from Scripts/Windows/MinGW/)
cd /d "%~dp0\..\..\.."

if not exist "Makefile" (
    echo [ERROR] Makefile not found. Please run GenerateProjectFiles.bat first.
    pause
    exit /b 1
)

:: ========== Locate MinGW / MSYS2 / GCC dynamically via PATH, Environment & Drives ==========
set "GCC_PATH="

:: 1. Check system PATH first
for /f "tokens=*" %%i in ('where gcc 2^>nul') do (
    if not defined GCC_PATH set "GCC_PATH=%%~dpi"
)

:: 2. Check standard environment variables
if not defined GCC_PATH (
    if defined MINGW_HOME   if exist "%MINGW_HOME%\bin\gcc.exe"          set "GCC_PATH=%MINGW_HOME%\bin"
    if defined MINGW64_HOME if exist "%MINGW64_HOME%\bin\gcc.exe"       set "GCC_PATH=%MINGW64_HOME%\bin"
    if defined MSYS2_HOME   if exist "%MSYS2_HOME%\mingw64\bin\gcc.exe" set "GCC_PATH=%MSYS2_HOME%\mingw64\bin"
    if defined MSYS_HOME    if exist "%MSYS_HOME%\mingw64\bin\gcc.exe"  set "GCC_PATH=%MSYS_HOME%\mingw64\bin"
)

:: 3. Dynamically discover all active system drives without hardcoding drive letters
if not defined GCC_PATH (
    for /f "tokens=*" %%d in ('powershell -NoProfile -Command "[System.IO.DriveInfo]::GetDrives() | Where-Object { $_.IsReady } | ForEach-Object { $_.Name.TrimEnd('\') }" 2^>nul') do (
        for %%s in (
            "%%d\MinGW\bin"
            "%%d\mingw64\bin"
            "%%d\mingw32\bin"
            "%%d\msys64\mingw64\bin"
            "%%d\msys64\ucrt64\bin"
            "%%d\msys64\mingw32\bin"
            "%%d\tools\msys64\mingw64\bin"
            "%%d\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin"
        ) do (
            if not defined GCC_PATH if exist "%%~s\gcc.exe" (
                set "GCC_PATH=%%~s"
            )
        )
    )
)

if defined GCC_PATH (
    echo [INFO] Found MinGW GCC at "%GCC_PATH%"
    set "PATH=%GCC_PATH%;%PATH%"
) else (
    echo [ERROR] MinGW GCC was not found in PATH, environment variables, or on any drive.
    echo [INFO] Please ensure gcc is available in your PATH or set MINGW_HOME.
    pause
    exit /b 1
)

:: Locate make executable dynamically
set "MAKE_EXE=mingw32-make"
where mingw32-make >nul 2>&1
if %errorlevel% neq 0 (
    where make >nul 2>&1
    if %errorlevel% equ 0 set "MAKE_EXE=make"
)

echo [INFO] Running %MAKE_EXE%...
"%MAKE_EXE%" SHELL=cmd.exe config=debug -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully.
pause
