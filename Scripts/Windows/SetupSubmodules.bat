@echo off
setlocal enabledelayedexpansion

echo [=== Setting up TimeEngine Submodules ^& Tools ===]
cd /d "%~dp0\..\.."

if not exist ".git" (
    echo [ERROR] This script must be run inside a Git repository. .git folder not found.
    pause
    exit /b 1
)

echo [INFO] Syncing submodule URLs...
git submodule sync --recursive

echo [INFO] Initializing submodules...
git submodule init

echo [INFO] Updating submodules recursively...
git submodule update --init --recursive --force

if %errorlevel% neq 0 (
    echo [ERROR] Failed to update submodules. Please check your internet connection and git configuration.
    pause
    exit /b 1
)

echo [SUCCESS] Git submodules updated successfully!

set "PREMAKE_DIR=%~dp0\..\..\ThirdParty\Premake\Windows"
set "PREMAKE_EXE=%PREMAKE_DIR%\premake5.exe"

if not exist "%PREMAKE_EXE%" (
    echo [INFO] Downloading Premake 5 for Windows...
    if not exist "%PREMAKE_DIR%" mkdir "%PREMAKE_DIR%"
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/premake/premake-core/releases/download/v5.0.0-beta4/premake-5.0.0-beta4-windows.zip' -OutFile '%PREMAKE_DIR%\premake.zip'"
    powershell -Command "Expand-Archive -Path '%PREMAKE_DIR%\premake.zip' -DestinationPath '%PREMAKE_DIR%' -Force"
    del "%PREMAKE_DIR%\premake.zip" 2>nul
    if exist "%PREMAKE_EXE%" (
        echo [SUCCESS] Premake 5 successfully installed to %PREMAKE_EXE%
    ) else (
        echo [WARN] Could not download Premake 5 binary. System premake5 will be used if available.
    )
) else (
    echo [INFO] Premake 5 binary verified at %PREMAKE_EXE%
)

echo [SUCCESS] Setup complete! You can now run GenerateProjectFiles to build the workspace.
