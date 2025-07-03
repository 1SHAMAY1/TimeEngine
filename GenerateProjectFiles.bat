@echo off
setlocal enabledelayedexpansion

echo [Cleaning build artifacts and Visual Studio files...]

:: Remove build output directories
rd /s /q Bin >nul 2>&1
rd /s /q Bin-Intermediate >nul 2>&1
rd /s /q .vs >nul 2>&1

:: Delete all solution and project files recursively
for /r %%f in (*.sln) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj.filters) do del /f /q "%%f" >nul 2>&1
for /r %%f in (*.vcxproj.user) do del /f /q "%%f" >nul 2>&1

:: Run Premake to regenerate the solution
echo [Generating new solution and project files using Premake...]
cd /d "%~dp0"
Vendor\bin\Premake\premake5.exe vs2022

if %errorlevel% neq 0 (
    echo [ERROR] Premake failed.
    pause
    exit /b 1
)

echo [Premake generation complete.]
pause
