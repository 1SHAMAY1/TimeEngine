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

echo [INFO] Running mingw32-make...
mingw32-make SHELL=cmd.exe config=debug -j2
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully.
pause
