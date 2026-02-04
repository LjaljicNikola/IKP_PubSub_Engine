@echo off
REM Quick Setup Test for PubSub Project
echo ================================================
echo PubSub Project - Setup Verification
echo ================================================
echo.

REM Test 1: Check if g++ is installed
echo [1/3] Checking g++ installation...
where g++ >nul 2>&1
if errorlevel 1 (
    echo [ERROR] g++ not found!
    echo.
    echo You need to install MinGW or MSYS2 first.
    echo Options:
    echo   1. Download MinGW-w64: https://winlibs.com/
    echo   2. Install MSYS2: https://www.msys2.org/
    echo.
    echo After installation, add the bin folder to your PATH.
    pause
    exit /b 1
) else (
    echo [OK] g++ found!
    g++ --version | findstr /C:"g++"
)
echo.

REM Test 2: Check C++17 support
echo [2/3] Checking C++17 support...
echo int main() { return 0; } > test_cpp17.cpp
g++ -std=c++17 test_cpp17.cpp -o test_cpp17.exe 2>nul
if errorlevel 1 (
    echo [ERROR] C++17 not supported!
    echo Please update your g++ compiler to version 7 or newer.
    del test_cpp17.cpp 2>nul
    pause
    exit /b 1
) else (
    echo [OK] C++17 supported!
    del test_cpp17.cpp test_cpp17.exe 2>nul
)
echo.

REM Test 3: Check pthread support
echo [3/3] Checking pthread support...
echo #include ^<thread^> > test_thread.cpp
echo int main() { return 0; } >> test_thread.cpp
g++ -std=c++17 -pthread test_thread.cpp -o test_thread.exe 2>nul
if errorlevel 1 (
    echo [WARNING] pthread might have issues
    echo Make sure you're using MinGW-w64 (not old MinGW)
    del test_thread.cpp 2>nul
) else (
    echo [OK] pthread supported!
    del test_thread.cpp test_thread.exe 2>nul
)
echo.

echo ================================================
echo Setup verification complete!
echo ================================================
echo.
echo You're ready to compile the project!
echo.
echo Quick commands:
echo   compile.bat              - Compile project
echo   pubsub_demo.exe          - Run compiled program
echo.
echo Or in VS Code:
echo   Ctrl+Shift+B             - Build
echo   F5                       - Debug
echo.
pause
