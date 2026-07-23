@echo off
title Building TimeEngine Commercial Release (MinGW Dist)
echo [=== Building TimeEngine Commercial Release (MinGW Dist) ===]

:: Go to root (3 directories back from Scripts/Windows/MinGW/)
cd /d "%~dp0\..\..\.."
set "ROOT_DIR=%CD%"

:: Check for Makefile
if not exist "Makefile" (
    echo [ERROR] Makefile not found. Please run MinGW\GenerateProjectFiles.bat first to generate the makefiles.
    pause
    exit /b 1
)

:: Build Solution using mingw32-make
echo [=== Building Makefile (Dist)... ===]
echo [Note: This may take a few minutes...]
mingw32-make SHELL=cmd.exe config=dist -j2
if %errorlevel% neq 0 (
    echo [ERROR] Build Failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build Successful!

:: Packaging
set "DIST_DIR=%ROOT_DIR%\Build_Release"
echo [=== Packaging to: %DIST_DIR% ===]

if exist "%DIST_DIR%" rd /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

:: Copy Exe and DLLs
:: MinGW outputs to Bin/Dist-windows-x86_64/TimeEditor/
set "BUILD_OUTPUT=%ROOT_DIR%\Bin\Dist-windows-x86_64\TimeEditor"

if not exist "%BUILD_OUTPUT%\TimeEditor.exe" (
    echo [ERROR] Build output not found at %BUILD_OUTPUT%\TimeEditor.exe
    pause
    exit /b 1
)

echo [INFO] Copying Binaries...
xcopy /s /y /q "%BUILD_OUTPUT%\*.*" "%DIST_DIR%\" >nul

:: Copy Resources
echo [INFO] Copying Assets and Resources...
if exist "%ROOT_DIR%\Resources" (
    xcopy /s /y /i /q "%ROOT_DIR%\Resources" "%DIST_DIR%\Resources" >nul
) else (
    echo [WARNING] Resources folder not found at %ROOT_DIR%\Resources
)

:: Create a Run script for convenience
echo @echo off > "%DIST_DIR%\RunEngine.bat"
echo start "" "TimeEditor.exe" >> "%DIST_DIR%\RunEngine.bat"

echo.
echo [SUCCESS] Packaging Complete!
echo [Executable located at: %DIST_DIR%\TimeEditor.exe]
echo.
pause
