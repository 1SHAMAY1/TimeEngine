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

:: ========== Locate MinGW / MSYS2 / GCC dynamically via PATH, Environment & Drives ==========
set "GCC_PATH="

:: 1. Check system PATH first
for /f "tokens=*" %%i in ('where gcc 2^>nul') do (
    if not defined GCC_PATH set "GCC_PATH=%%~dpi"
)

:: 2. Check standard environment variables
if not defined GCC_PATH (
    if defined MINGW_HOME   if exist "%MINGW_HOME%\bin\gcc.exe"          set "GCC_PATH=%MINGW_HOME%\bin"
    if defined MINGW64_HOME if exist "%MINGW64_HOME%\bin\gcc.exe"       set "GCC_PATH=%MINGW64_HOME%\bin"
    if defined MSYS2_HOME   if exist "%MSYS2_HOME%\mingw64\bin\gcc.exe" set "GCC_PATH=%MSYS2_HOME%\mingw64\bin"
    if defined MSYS_HOME    if exist "%MSYS_HOME%\mingw64\bin\gcc.exe"  set "GCC_PATH=%MSYS_HOME%\mingw64\bin"
)

:: 3. Dynamically discover all active system drives without hardcoding drive letters
if not defined GCC_PATH (
    for /f "tokens=*" %%d in ('powershell -NoProfile -Command "[System.IO.DriveInfo]::GetDrives() | Where-Object { $_.IsReady } | ForEach-Object { $_.Name.TrimEnd('\') }" 2^>nul') do (
        for %%s in (
            "%%d\MinGW\bin"
            "%%d\mingw64\bin"
            "%%d\mingw32\bin"
            "%%d\msys64\mingw64\bin"
            "%%d\msys64\ucrt64\bin"
            "%%d\msys64\mingw32\bin"
            "%%d\tools\msys64\mingw64\bin"
            "%%d\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin"
        ) do (
            if not defined GCC_PATH if exist "%%~s\gcc.exe" (
                set "GCC_PATH=%%~s"
            )
        )
    )
)

if defined GCC_PATH (
    echo [INFO] Found MinGW GCC at "%GCC_PATH%"
    set "PATH=%GCC_PATH%;%PATH%"
) else (
    echo [ERROR] MinGW GCC was not found in PATH, environment variables, or on any drive.
    echo [INFO] Please ensure gcc is available in your PATH or set MINGW_HOME.
    pause
    exit /b 1
)

:: Locate make executable dynamically
set "MAKE_EXE=mingw32-make"
where mingw32-make >nul 2>&1
if %errorlevel% neq 0 (
    where make >nul 2>&1
    if %errorlevel% equ 0 set "MAKE_EXE=make"
)

:: ========== Logger ==========
echo [=== CMake configure/build: Logger ===]
cd "%ROOT_DIR%\Vendor\Customizable_Logger"
if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_MAKE_PROGRAM="%MAKE_EXE%"
if %errorlevel% neq 0 (
    echo [ERROR] Logger CMake configuration failed.
    pause
    exit /b 1
)

"%MAKE_EXE%" SHELL=cmd.exe
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

cmake ../glfw -G "MinGW Makefiles" -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_MAKE_PROGRAM="%MAKE_EXE%"
if %errorlevel% neq 0 (
    echo [ERROR] GLFW CMake configuration failed.
    pause
    exit /b 1
)

"%MAKE_EXE%" SHELL=cmd.exe
if %errorlevel% neq 0 (
    echo [ERROR] GLFW build failed.
    pause
    exit /b 1
)
cd "%ROOT_DIR%"

:: ========== Premake ==========
echo [=== Generating Makefiles with Premake... ===]
Vendor\Premake\Windows\premake5.exe gmake2
if %errorlevel% neq 0 (
    echo [ERROR] Premake generation failed.
    pause
    exit /b 1
)

echo [SUCCESS] Setup complete. Run "mingw32-make SHELL=cmd.exe config=debug" to compile the solution.
pause
