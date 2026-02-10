@echo off
REM Script to start PubSub Engine, 2 Publishers, and 3 Subscribers

echo.
echo ========================================
echo Starting PubSub Distributed System
echo ========================================
echo.

REM Build the executable first
echo [1/6] Building project...
call compile.bat
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo [2/6] Starting PubSub Engine on port 5000...
start "PubSub Engine" cmd /k "pubsub.exe --engine"

REM Wait for engine to start
timeout /t 2 /nobreak

echo [3/6] Starting Publisher 1 on port 4101...
start "Publisher 1" cmd /k "pubsub.exe --publisher --port 4101 --engine-host localhost --engine-port 5000"

echo [4/6] Starting Publisher 2 on port 4102...
start "Publisher 2" cmd /k "pubsub.exe --publisher --port 4102 --engine-host localhost --engine-port 5000"

REM Wait for publishers to connect
timeout /t 1 /nobreak

echo [5/6] Starting Subscriber 1 on port 4201 - Topic: Analog/MER/220...
start "Subscriber 1" cmd /k "pubsub.exe --subscriber --port 4201 --topic ""Analog/MER/220"" --engine-host localhost --engine-port 5000"

echo [6/6] Starting Subscriber 2 on port 4202 - Topic: Status/SWG/1...
start "Subscriber 2" cmd /k "pubsub.exe --subscriber --port 4202 --topic ""Status/SWG/1"" --engine-host localhost --engine-port 5000"

echo [7/7] Starting Subscriber 3 on port 4203 - Topic: Status/CRB/1...
start "Subscriber 3" cmd /k "pubsub.exe --subscriber --port 4203 --topic ""Status/CRB/1"" --engine-host localhost --engine-port 5000"

echo.
echo ========================================
echo All services started!
echo ========================================
echo.
echo Engine:       localhost:5000
echo Publisher 1:  localhost:4101
echo Publisher 2:  localhost:4102
echo Subscriber 1: localhost:4201 (listening to Analog/MER/220)
echo Subscriber 2: localhost:4202 (listening to Status/SWG/1)
echo Subscriber 3: localhost:4203 (listening to Status/CRB/1)
echo.
echo Type 'exit' in any window to gracefully shutdown that service.
echo ========================================
echo.
pause
