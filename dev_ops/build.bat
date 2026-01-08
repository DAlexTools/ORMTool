@echo off
setlocal enabledelayedexpansion

REM ==============================
REM Build script for ORMTool
REM ==============================

REM Prompt user for configuration
echo.
echo =====================================================
echo   Select build configuration:
echo   [1] Debug
echo   [2] Release
echo =====================================================
set /p choice=Enter your choice (1 or 2): 

REM Set config and build folder
set BUILD_SUBDIR=build

if "%choice%"=="1" (
    set CONFIG=Debug
) else if "%choice%"=="2" (
    set CONFIG=Release
) else (
    echo Invalid choice.
    pause
    exit /b 1
)

REM Set paths
set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set BUILD_DIR=%PROJECT_ROOT%\%BUILD_SUBDIR%

REM Show summary
echo.
echo =====================================================
echo Configuration:    %CONFIG%
echo Build directory:  %BUILD_DIR%
echo =====================================================

REM Create build directory if it doesn't exist
if not exist "!BUILD_DIR!" (
    mkdir "!BUILD_DIR!"
)

REM Go to build directory
cd /d "!BUILD_DIR!"

REM Generate Visual Studio project (multi-config)
cmake .. -G "Visual Studio 17 2022"

if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

REM Build selected configuration
cmake --build . --config %CONFIG%

if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

echo.
echo %CONFIG% build completed successfully.
pause
exit /b 0
