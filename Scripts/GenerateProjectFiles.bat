@echo off
title Generating TimeEngine Workspace
chcp 65001 > nul

echo [≡ Generating TimeEngine Workspace: Clean + CMake + Premake]

:: Go to root (1 directory back from Scripts/)
cd /d "%~dp0\.."
set "ROOT_DIR=%CD%"
set "SCRIPT_DIR=%ROOT_DIR%\Scripts"

:: ========== Cleanup ==========
echo [≡ Cleaning previous build artifacts...]
call "%SCRIPT_DIR%\CleanProjectFiles.bat"

:: ========== Logger ==========
echo [≡ CMake configure/build: Logger]
cd "%ROOT_DIR%\Logger\Customizable_Logger"
if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo [✖ Logger CMake configuration failed.]
    pause
    exit /b 1
)

cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo [✖ Logger build failed.]
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== GLFW ==========
echo [≡ CMake configure/build: GLFW]
cd "%ROOT_DIR%\Vendor\GLFW"
if not exist build mkdir build
cd build

cmake ../glfw -G "Visual Studio 17 2022" -A x64 -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF
if %errorlevel% neq 0 (
    echo [✖ GLFW CMake configuration failed.]
    pause
    exit /b 1
)

cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo [✖ GLFW build failed.]
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== Premake ==========
echo [≡ Generating Visual Studio solution with Premake...]
cd "%ROOT_DIR%"
Vendor\Premake\premake5.exe vs2022
if %errorlevel% neq 0 (
    echo [✖ Premake generation failed.]
    pause
    exit /b 1
)



echo [✅ Setup complete. Open TimeEngine.sln from root to begin development.]
pause
