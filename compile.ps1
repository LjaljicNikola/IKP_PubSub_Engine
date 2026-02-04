# PowerShell compile script for PubSub Project
# Requires MinGW or similar compiler with g++

Write-Host "Compiling PubSub Project..." -ForegroundColor Cyan
Write-Host ""

# Clean previous build
if (Test-Path "pubsub_demo.exe") { Remove-Item "pubsub_demo.exe" }
if (Test-Path "src/main.o") { Remove-Item "src/main.o" }
if (Test-Path "src/Publisher.o") { Remove-Item "src/Publisher.o" }
if (Test-Path "src/Subscriber.o") { Remove-Item "src/Subscriber.o" }
if (Test-Path "src/PubSubEngine.o") { Remove-Item "src/PubSubEngine.o" }

# Compile all source files
Write-Host "Compiling main.cpp..." -ForegroundColor Yellow
& g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/main.cpp -o src/main.o
if ($LASTEXITCODE -ne 0) { 
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1 
}

Write-Host "Compiling Publisher.cpp..." -ForegroundColor Yellow
& g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/Publisher.cpp -o src/Publisher.o
if ($LASTEXITCODE -ne 0) { 
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1 
}

Write-Host "Compiling Subscriber.cpp..." -ForegroundColor Yellow
& g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/Subscriber.cpp -o src/Subscriber.o
if ($LASTEXITCODE -ne 0) { 
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1 
}

Write-Host "Compiling PubSubEngine.cpp..." -ForegroundColor Yellow
& g++ -std=c++17 -Wall -Wextra -pthread -I./src -c src/PubSubEngine.cpp -o src/PubSubEngine.o
if ($LASTEXITCODE -ne 0) { 
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1 
}

# Link all object files
Write-Host ""
Write-Host "Linking..." -ForegroundColor Yellow
& g++ -std=c++17 -Wall -Wextra -pthread -o pubsub_demo.exe src/main.o src/Publisher.o src/Subscriber.o src/PubSubEngine.o
if ($LASTEXITCODE -ne 0) { 
    Write-Host "Build FAILED!" -ForegroundColor Red
    exit 1 
}

Write-Host ""
Write-Host "===================================" -ForegroundColor Green
Write-Host "Build complete!" -ForegroundColor Green
Write-Host "Run with: .\pubsub_demo.exe" -ForegroundColor Green
Write-Host "===================================" -ForegroundColor Green
