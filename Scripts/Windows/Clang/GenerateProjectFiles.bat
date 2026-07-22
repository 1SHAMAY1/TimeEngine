@echo off
title Generating TimeEngine Workspace (ClangCL)

echo [=== Generating TimeEngine Workspace: Clean + CMake + Premake (ClangCL) ===]

:: Go to root (3 directories back from Scripts/Windows/Clang/)
cd /d "%~dp0\..\..\.."
set "ROOT_DIR=%CD%"
set "SCRIPT_DIR=%ROOT_DIR%\Scripts\Windows\Clang"

:: ========== Clean ==========
echo [INFO] Cleaning previous build artifacts, CMake, and Visual Studio files...

rd /s /q "%ROOT_DIR%\Bin" >nul 2>&1
rd /s /q "%ROOT_DIR%\Bin-Intermediate" >nul 2>&1

rd /s /q "%ROOT_DIR%\Vendor\Customizable_Logger\build" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\Customizable_Logger\bin" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\Customizable_Logger\lib" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\Customizable_Logger\CMakeCache.txt" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\Customizable_Logger\CMakeFiles" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\Customizable_Logger\Makefile" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\Customizable_Logger\cmake_install.cmake" >nul 2>&1

rd /s /q "%ROOT_DIR%\Vendor\GLFW\build" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\bin" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\CMakeCache.txt" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\GLFW\CMakeFiles" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\Makefile" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\cmake_install.cmake" >nul 2>&1

for /r "%ROOT_DIR%" %%f in (*.sln) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj.filters) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj.user) do del /f /q "%%f" >nul 2>&1

for /r "%ROOT_DIR%" %%f in (Makefile) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.make) do del /f /q "%%f" >nul 2>&1

rd /s /q "%ROOT_DIR%\.vs" >nul 2>&1

echo [SUCCESS] Cleanup complete.

:: ========== Logger ==========
echo [=== CMake configure/build: Logger ===]
cd "%ROOT_DIR%\Vendor\Customizable_Logger"
if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -T ClangCL -A x64
if %errorlevel% neq 0 (
    echo [ERROR] Logger CMake configuration failed.
    pause
    exit /b 1
)

cmake --build . --config Debug -- /m:1
if %errorlevel% neq 0 (
    echo [ERROR] Logger build failed.
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== GLFW ==========
echo [=== CMake configure/build: GLFW ===]
cd "%ROOT_DIR%\Vendor\GLFW"
if not exist build mkdir build
cd build

cmake ../glfw -G "Visual Studio 17 2022" -T ClangCL -A x64 -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF
if %errorlevel% neq 0 (
    echo [ERROR] GLFW CMake configuration failed.
    pause
    exit /b 1
)

cmake --build . --config Debug -- /m:1
if %errorlevel% neq 0 (
    echo [ERROR] GLFW build failed.
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== Premake ==========
echo [=== Generating Visual Studio solution with Premake... ===]
Vendor\Premake\premake5.exe vs2022
if %errorlevel% neq 0 (
    echo [ERROR] Premake generation failed.
    pause
    exit /b 1
)

echo [SUCCESS] Setup complete. Open TimeEngine.sln and build with ClangCL toolset, or run Clang\BuildDebug.bat.
pause
