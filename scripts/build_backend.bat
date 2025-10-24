@echo off
echo Building HW Analyzer Backend...

REM Get the script's directory and navigate to project root
cd /d "%~dp0.."

REM Clean build directory if it exists (force delete)
if exist backend\build (
    echo Cleaning old build directory...
    timeout /t 1 /nobreak >nul
    rd /s /q backend\build 2>nul
    if exist backend\build (
        echo Warning: Could not fully clean build directory, trying to continue...
    )
)

if not exist backend\build mkdir backend\build
cd backend\build

REM Let CMake auto-detect the best generator
echo Configuring with CMake...
cmake ..
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo Building...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Backend built successfully!
    echo Binary location: backend\build\Release\hw_analyzer_backend.exe
    echo ========================================
) else (
    echo Build failed!
    exit /b 1
)

if %ERRORLEVEL% EQU 0 (
    echo Backend built successfully!
    echo Binary location: backend\build\Release\hw_analyzer_backend.exe
) else (
    echo Build failed!
    exit /b 1
)
