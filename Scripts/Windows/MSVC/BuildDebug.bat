@echo off
title Building TimeEngine (MSVC Debug)
echo [=== Building TimeEngine (MSVC Debug) ===]

:: Go to root (3 directories back from Scripts/Windows/MSVC/)
cd /d "%~dp0\..\..\.."

if not exist "TimeEngine.sln" (
    echo [ERROR] Solution file not found. Please run GenerateProjectFiles.bat first.
    pause
    exit /b 1
)

:: Locate vswhere dynamically via environment, registry & mounted drives
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
    echo [ERROR] vswhere.exe not found on any drive or PATH.
    pause
    exit /b 1
)

set "VS_INSTALL_DIR="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    if exist "%%i\Common7\Tools\VsDevCmd.bat" set "VS_INSTALL_DIR=%%i"
)
if "%VS_INSTALL_DIR%"=="" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
        if exist "%%i\Common7\Tools\VsDevCmd.bat" set "VS_INSTALL_DIR=%%i"
    )
)

if "%VS_INSTALL_DIR%"=="" (
    echo [ERROR] Could not locate a Visual Studio installation with MSBuild.
    pause
    exit /b 1
)

if exist "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" (
    if "%VSCMD_VER%"=="" (
        echo [INFO] Found Visual Studio at "%VS_INSTALL_DIR%"
        echo [INFO] Initializing MSVC build environment...
        call "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo
    )
)

:: Dynamically detect installed MSVC Platform Toolset from VCToolsVersion (e.g. 14.51 -> v145, 14.38 -> v143, 15.01 -> v150)
set "TOOLSET_ARG="
if defined VCToolsVersion (
    for /f "tokens=1,2 delims=." %%a in ("%VCToolsVersion%") do (
        set "MAJOR_VER=%%a"
        set "MINOR_VER=%%b"
    )
)
if defined MAJOR_VER if defined MINOR_VER (
    set "TOOLSET_ARG=/p:PlatformToolset=v%MAJOR_VER%%MINOR_VER:~0,1%"
)

echo [INFO] Running MSBuild...
msbuild TimeEngine.sln /p:Configuration=Debug /p:Platform=x64 %TOOLSET_ARG% /m
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully.
pause
