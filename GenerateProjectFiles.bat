@echo off
setlocal enabledelayedexpansion

echo [Cleaning build artifacts and Visual Studio files...]

:: Clean general build artifacts
rd /s /q Bin >nul 2>&1
rd /s /q Bin-Intermediate >nul 2>&1
rd /s /q .vs >nul 2>&1

:: Clean Logger CMake build artifacts
rd /s /q Logger\Customizable_Logger\build >nul 2>&1
rd /s /q Logger\Customizable_Logger\lib >nul 2>&1
rd /s /q Logger\Customizable_Logger\bin >nul 2>&1

:: Clean GLFW build artifacts
rd /s /q Vendor\bin >nul 2>&1
rd /s /q Vendor\GLFW\build >nul 2>&1

:: Delete all VS project/solution files
for /r %%f in (*.sln) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj.filters) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj.user) do del /f /q "%%f" >nul 2>&1

:: ===========================
:: Build Logger (CMake Static)
:: ===========================
echo [Running CMake to build Logger...]
cd Logger\Customizable_Logger

mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration for Logger failed.
    pause
    exit /b 1
)

cmake --build . --config Debug

if %errorlevel% neq 0 (
    echo [ERROR] Building Logger with CMake failed.
    pause
    exit /b 1
)

cd ..\..\..

:: ===========================
:: Build GLFW (CMake Static)
:: ===========================
echo [Running CMake to build GLFW...]
mkdir Vendor\GLFW\build
cd Vendor\GLFW\build

cmake ../glfw -G "Visual Studio 17 2022" -A x64 -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration for GLFW failed.
    pause
    exit /b 1
)

cmake --build . --config Debug

if %errorlevel% neq 0 (
    echo [ERROR] Building GLFW with CMake failed.
    pause
    exit /b 1
)

cd ..\..\..

:: ===========================
:: Generate Solution (Premake)
:: ===========================
echo [Running Premake to generate TimeEngine solution...]
Vendor\Premake\premake5.exe vs2022

if %errorlevel% neq 0 (
    echo [ERROR] Premake generation failed.
    pause
    exit /b 1
)

echo [Setup complete. Open TimeEngine.sln to begin development.]
pause
