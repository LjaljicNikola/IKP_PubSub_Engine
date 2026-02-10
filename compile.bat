@echo off
REM Compile script for PubSub Distributed Project
REM Requires MinGW g++ with C++17 support and winsock2

echo.
echo Compiling PubSub Distributed Project...
echo ========================================

REM Clean previous build (remove object files but keep exe)
echo Cleaning previous build...
for /r "src" %%f in (*.o) do del "%%f" 2>nul

REM Compile individual source files
echo.
echo Compiling...

set CXXFLAGS=-std=c++17 -Wall -Wextra -pthread -I./src

echo Compiling src/main.cpp...
g++ %CXXFLAGS% -c src/main.cpp -o src/main.o
if errorlevel 1 goto :error

echo Compiling src/Network.cpp...
g++ %CXXFLAGS% -c src/Network.cpp -o src/Network.o
if errorlevel 1 goto :error

echo Compiling src/core/Publisher.cpp...
g++ %CXXFLAGS% -c src/core/Publisher.cpp -o src/core/Publisher.o
if errorlevel 1 goto :error

echo Compiling src/core/Subscriber.cpp...
g++ %CXXFLAGS% -c src/core/Subscriber.cpp -o src/core/Subscriber.o
if errorlevel 1 goto :error

echo Compiling src/core/PubSubEngine.cpp...
g++ %CXXFLAGS% -c src/core/PubSubEngine.cpp -o src/core/PubSubEngine.o
if errorlevel 1 goto :error

echo Compiling src/utils/MessageValidator.cpp...
g++ %CXXFLAGS% -c src/utils/MessageValidator.cpp -o src/utils/MessageValidator.o
if errorlevel 1 goto :error

echo Compiling src/utils/CommandLineParser.cpp...
g++ %CXXFLAGS% -c src/utils/CommandLineParser.cpp -o src/utils/CommandLineParser.o
if errorlevel 1 goto :error

echo Compiling src/utils/NetworkUtils.cpp...
g++ %CXXFLAGS% -c src/utils/NetworkUtils.cpp -o src/utils/NetworkUtils.o
if errorlevel 1 goto :error

echo Compiling src/utils/MessageFormatter.cpp...
g++ %CXXFLAGS% -c src/utils/MessageFormatter.cpp -o src/utils/MessageFormatter.o
if errorlevel 1 goto :error

REM Link all object files
echo.
echo Linking...
g++ %CXXFLAGS% -o pubsub.exe src/main.o src/Network.o src/core/Publisher.o src/core/Subscriber.o src/core/PubSubEngine.o src/utils/MessageValidator.o src/utils/CommandLineParser.o src/utils/NetworkUtils.o src/utils/MessageFormatter.o -lws2_32
if errorlevel 1 goto :error

echo.
echo ========================================
echo Build complete!
echo ========================================
echo.
echo Run services with:
echo   Engine:     .\pubsub.exe --engine
echo   Publisher:  .\pubsub.exe --publisher
echo   Subscriber: .\pubsub.exe --subscriber --topic "Analog/MER/220"
echo.
exit /b 0

:error
echo.
echo ========================================
echo Build FAILED!
echo ========================================
exit /b 1
