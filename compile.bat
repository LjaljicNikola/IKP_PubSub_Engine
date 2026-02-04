@echo off
REM Compile script for PubSub Project on Windows
REM Requires MinGW or similar compiler with g++

echo Compiling PubSub Project...
echo.

REM Clean previous build
if exist pubsub_demo.exe del pubsub_demo.exe
if exist src\main.o del src\main.o
if exist src\Publisher.o del src\Publisher.o
if exist src\Subscriber.o del src\Subscriber.o
if exist src\PubSubEngine.o del src\PubSubEngine.o

REM Compile all source files
echo Compiling main.cpp...
g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/main.cpp -o src/main.o
if errorlevel 1 goto error

echo Compiling Publisher.cpp...
g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/Publisher.cpp -o src/Publisher.o
if errorlevel 1 goto error

echo Compiling Subscriber.cpp...
g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/Subscriber.cpp -o src/Subscriber.o
if errorlevel 1 goto error

echo Compiling PubSubEngine.cpp...
g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/PubSubEngine.cpp -o src/PubSubEngine.o
if errorlevel 1 goto error

REM Link all object files
echo.
echo Linking...
g++ -std=c++17 -Wall -Wextra -pthread -o pubsub_demo.exe src/main.o src/Publisher.o src/Subscriber.o src/PubSubEngine.o
if errorlevel 1 goto error

echo.
echo ===================================
echo Build complete! 
echo Run with: pubsub_demo.exe
echo ===================================
goto end

:error
echo.
echo ===================================
echo Build FAILED!
echo ===================================
exit /b 1

:end
