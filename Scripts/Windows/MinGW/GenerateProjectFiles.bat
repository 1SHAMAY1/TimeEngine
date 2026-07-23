@echo off
title Generating TimeEngine Workspace (MinGW)

echo [=== Generating TimeEngine Workspace: Clean + CMake + Premake (MinGW) ===]

:: Go to root (3 directories back from Scripts/Windows/MinGW/)
cd /d "%~dp0\..\..\.."
set "ROOT_DIR=%CD%"
set "SCRIPT_DIR=%ROOT_DIR%\Scripts\Windows\MinGW"

:: ========== Clean ==========
echo [INFO] Cleaning previous build artifacts, CMake, and Makefile/project files...

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

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
if %errorlevel% neq 0 (
    echo [ERROR] Logger CMake configuration failed.
    pause
    exit /b 1
)

mingw32-make SHELL=cmd.exe
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

cmake ../glfw -G "MinGW Makefiles" -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug
if %errorlevel% neq 0 (
    echo [ERROR] GLFW CMake configuration failed.
    pause
    exit /b 1
)

mingw32-make SHELL=cmd.exe
if %errorlevel% neq 0 (
    echo [ERROR] GLFW build failed.
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== Premake ==========
echo [=== Generating Makefiles with Premake... ===]
Vendor\Premake\premake5.exe gmake2
if %errorlevel% neq 0 (
    echo [ERROR] Premake generation failed.
    pause
    exit /b 1
)

echo [SUCCESS] Setup complete. Run "mingw32-make SHELL=cmd.exe config=debug" to compile the solution.
pause
