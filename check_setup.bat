@echo off
REM Check setup for PubSub Project
REM Verifies required tools are installed and accessible

echo.
echo Checking PubSub Project Setup...
echo ================================

echo.
echo [1] Checking for g++ compiler...
where g++ >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] g++ found
    g++ --version | findstr /R ".*"
) else (
    echo [FAIL] g++ not found in PATH
    echo        MinGW is required. Install from: https://www.mingw-w64.org/
    exit /b 1
)

echo.
echo [2] Checking for C++17 support...
g++ -std=c++17 -dumpversion >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] C++17 support available
) else (
    echo [FAIL] C++17 not supported
    exit /b 1
)

echo.
echo [3] Checking project structure...
if exist "src" (
    echo [OK] src/ directory found
) else (
    echo [FAIL] src/ directory not found
    exit /b 1
)

if exist "src\core" (
    echo [OK] src/core/ directory found
) else (
    echo [FAIL] src/core/ directory not found
    exit /b 1
)

if exist "src\utils" (
    echo [OK] src/utils/ directory found
) else (
    echo [FAIL] src/utils/ directory not found
    exit /b 1
)

echo.
echo ================================
echo Setup check complete!
echo.
echo Run 'compile.bat' to build the project.
echo.
