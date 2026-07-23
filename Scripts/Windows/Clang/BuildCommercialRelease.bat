@echo off
title Building TimeEngine Commercial Release (Clang Dist)
echo [=== Building TimeEngine Commercial Release (Clang Dist) ===]

:: Go to root (3 directories back from Scripts/Windows/Clang/)
cd /d "%~dp0\..\..\.."
set "ROOT_DIR=%CD%"

:: Check for Solution
if not exist "TimeEngine.sln" (
    echo [ERROR] Solution file not found. Please run Clang\GenerateProjectFiles.bat first.
    pause
    exit /b 1
)

:: Find MSBuild path using vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [ERROR] vswhere.exe not found at default location.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_INSTALL_DIR=%%i"
)

if "%VS_INSTALL_DIR%"=="" (
    echo [ERROR] Could not locate a Visual Studio installation with MSBuild.
    pause
    exit /b 1
)

if not exist "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" (
    echo [ERROR] VsDevCmd.bat not found in "%VS_INSTALL_DIR%".
    pause
    exit /b 1
)

echo [INFO] Found Visual Studio at: %VS_INSTALL_DIR%
echo [INFO] Setting up Build Environment...
call "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo

:: Build Solution
echo [=== Building TimeEngine.sln (Clang Dist|x64)... ===]
echo [Note: This may take a few minutes...]
msbuild TimeEngine.sln /p:Configuration=Dist /p:Platform=x64 /p:PlatformToolset=ClangCL /t:Rebuild /m
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
