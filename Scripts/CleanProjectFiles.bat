@echo off
title Cleaning TimeEngine Workspace
chcp 65001 >nul

echo [🧹 Cleaning TimeEngine build artifacts, CMake, and Visual Studio files...]

:: Change to root directory from Scripts/
cd /d "%~dp0\.."
set "ROOT_DIR=%CD%"

:: === Root build folders ===
rd /s /q "%ROOT_DIR%\Bin" >nul 2>&1
rd /s /q "%ROOT_DIR%\Bin-Intermediate" >nul 2>&1

:: === Logger CMake cleanup ===
rd /s /q "%ROOT_DIR%\Logger\Customizable_Logger\build" >nul 2>&1
rd /s /q "%ROOT_DIR%\Logger\Customizable_Logger\bin" >nul 2>&1
rd /s /q "%ROOT_DIR%\Logger\Customizable_Logger\lib" >nul 2>&1
del /f /q "%ROOT_DIR%\Logger\Customizable_Logger\CMakeCache.txt" >nul 2>&1
rd /s /q "%ROOT_DIR%\Logger\Customizable_Logger\CMakeFiles" >nul 2>&1
del /f /q "%ROOT_DIR%\Logger\Customizable_Logger\Makefile" >nul 2>&1
del /f /q "%ROOT_DIR%\Logger\Customizable_Logger\cmake_install.cmake" >nul 2>&1

:: === GLFW CMake cleanup ===
rd /s /q "%ROOT_DIR%\Vendor\GLFW\build" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\bin" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\CMakeCache.txt" >nul 2>&1
rd /s /q "%ROOT_DIR%\Vendor\GLFW\CMakeFiles" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\Makefile" >nul 2>&1
del /f /q "%ROOT_DIR%\Vendor\GLFW\cmake_install.cmake" >nul 2>&1

:: === Visual Studio generated files (from entire tree) ===
for /r "%ROOT_DIR%" %%f in (*.sln) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj.filters) do del /f /q "%%f" >nul 2>&1
for /r "%ROOT_DIR%" %%f in (*.vcxproj.user) do del /f /q "%%f" >nul 2>&1

:: === .vs hidden folder ===
rd /s /q "%ROOT_DIR%\.vs" >nul 2>&1

echo [✅ Cleanup complete. All build, CMake, and Visual Studio artifacts removed.]
pause
