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
