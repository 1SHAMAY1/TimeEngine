@echo off
title Building TimeEngine (Clang Debug)
echo [=== Building TimeEngine (Clang Debug) ===]

:: Go to root (3 directories back from Scripts/Windows/Clang/)
cd /d "%~dp0\..\..\.."

if not exist "TimeEngine.sln" (
    echo [ERROR] Solution file not found. Please run GenerateProjectFiles.bat first.
    pause
    exit /b 1
)

:: Find MSBuild path using vswhere dynamically via environment, registry & mounted drives
set "VSWHERE="
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if "%VSWHERE%"=="" if exist "%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if "%VSWHERE%"=="" (
    for /f "tokens=*" %%i in ('where vswhere 2^>nul') do set "VSWHERE=%%i"
)
if "%VSWHERE%"=="" (
    for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\vswhere.exe" /ve 2^>nul ^| findstr /i "REG_"') do (
        if exist "%%b" set "VSWHERE=%%b"
    )
)
if "%VSWHERE%"=="" (
    for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\App Paths\vswhere.exe" /ve 2^>nul ^| findstr /i "REG_"') do (
        if exist "%%b" set "VSWHERE=%%b"
    )
)
if "%VSWHERE%"=="" (
    for /f "tokens=1*" %%a in ('fsutil fsinfo drives 2^>nul') do (
        for %%d in (%%b) do (
            if "%VSWHERE%"=="" if exist "%%dProgram Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%%dProgram Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
            if "%VSWHERE%"=="" if exist "%%dProgram Files\Microsoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%%dProgram Files\Microsoft Visual Studio\Installer\vswhere.exe"
            if "%VSWHERE%"=="" if exist "%%dVisualStudio\Installer\vswhere.exe" set "VSWHERE=%%dVisualStudio\Installer\vswhere.exe"
            if "%VSWHERE%"=="" if exist "%%dMicrosoft Visual Studio\Installer\vswhere.exe" set "VSWHERE=%%dMicrosoft Visual Studio\Installer\vswhere.exe"
        )
    )
)

if "%VSWHERE%"=="" (
    echo [ERROR] vswhere.exe not found on any drive or registry.
    pause
    exit /b 1
)

set "VS_INSTALL_DIR="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    if exist "%%i\MSBuild\Current\Bin\amd64\MSBuild.exe" set "VS_INSTALL_DIR=%%i"
    if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "VS_INSTALL_DIR=%%i"
)
if "%VS_INSTALL_DIR%"=="" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
        if exist "%%i\MSBuild\Current\Bin\amd64\MSBuild.exe" set "VS_INSTALL_DIR=%%i"
        if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "VS_INSTALL_DIR=%%i"
    )
)

if "%VS_INSTALL_DIR%"=="" (
    echo [ERROR] Could not locate a Visual Studio installation with MSBuild.
    pause
    exit /b 1
)

:: Set up path to MSBuild.exe
set "MSBUILD_PATH=%VS_INSTALL_DIR%\MSBuild\Current\Bin\amd64\MSBuild.exe"
if not exist "%MSBUILD_PATH%" (
    set "MSBUILD_PATH=%VS_INSTALL_DIR%\MSBuild\Current\Bin\MSBuild.exe"
)

echo [INFO] Running MSBuild with ClangCL toolset...
"%MSBUILD_PATH%" TimeEngine.sln /p:Configuration=Debug /p:Platform=x64 /p:PlatformToolset=ClangCL /m
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully.
pause
