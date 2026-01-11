@echo off
title Building TimeEngine Commercial Release (Distribution)
echo [≡ Building TimeEngine Commercial Release (Dist)]

:: Go to root (1 directory back from Scripts/)
cd /d "%~dp0\.."
set "ROOT_DIR=%CD%"

:: Check for Solution
if not exist "TimeEngine.sln" (
    echo [!] Solution file not found. Please run GenerateProjectFiles.bat first to generate the project files.
    pause
    exit /b 1
)

:: Find Visual Studio 2022 using vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo [!] vswhere.exe not found at default location.
    echo [!] Please ensure Visual Studio Installer is installed in the default location or edit this script.
    pause
    exit /b 1
)

:: Get VS Installation Path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set "VS_INSTALL_DIR=%%i"
)

if "%VS_INSTALL_DIR%"=="" (
    echo [!] Could not locate a Visual Studio installation with MSBuild.
    pause
    exit /b 1
)

if not exist "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" (
    echo [!] VsDevCmd.bat not found in "%VS_INSTALL_DIR%".
    echo [!] Please verify your Visual Studio installation.
    pause
    exit /b 1
)

echo [Found Visual Studio at: %VS_INSTALL_DIR%]
echo [Setting up Build Environment...]
call "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo

:: Build Solution
echo [≡ Building TimeEngine.sln (Dist|x64)...]
echo [Note: This may take a few minutes...]
msbuild TimeEngine.sln /p:Configuration=Dist /p:Platform=x64 /t:Rebuild /m
if %errorlevel% neq 0 (
    echo [✖ Build Failed!]
    pause
    exit /b 1
)

echo.
echo [✅ Build Successful!]

:: Packaging
set "DIST_DIR=%ROOT_DIR%\Build_Release"
echo [≡ Packaging to: %DIST_DIR%]

if exist "%DIST_DIR%" rd /s /q "%DIST_DIR%"
mkdir "%DIST_DIR%"

:: Copy Exe and DLLs
:: 'Sandbox' project output contains the Engine.dll and Sandbox.exe from the build output
:: Path scheme: Bin\Dist-windows-x86_64\Sandbox
set "BUILD_OUTPUT=%ROOT_DIR%\Bin\Dist-windows-x86_64\Sandbox"

if not exist "%BUILD_OUTPUT%\Sandbox.exe" (
    echo [!] Error: Build output not found at %BUILD_OUTPUT%\Sandbox.exe
    pause
    exit /b 1
)

echo [  - Copying Binaries...]
xcopy /s /y /q "%BUILD_OUTPUT%\*.*" "%DIST_DIR%\" >nul

:: Copy Resources
:: Engine expects Resources folder in the execution directory (or one up, usually execution dir for distribution)
echo [  - Copying Assets & Resources...]
if exist "%ROOT_DIR%\Resources" (
    xcopy /s /y /i /q "%ROOT_DIR%\Resources" "%DIST_DIR%\Resources" >nul
) else (
    echo [!] Warning: Resources folder not found at %ROOT_DIR%\Resources
)

:: Create a Run script for convenience
echo @echo off > "%DIST_DIR%\RunEngine.bat"
echo start "" "Sandbox.exe" >> "%DIST_DIR%\RunEngine.bat"

echo.
echo [✅ Packaging Complete!]
echo [Executable located at: %DIST_DIR%\Sandbox.exe]
echo [You can now zip the '%DIST_DIR%' folder and distribute it.]
echo.
pause
