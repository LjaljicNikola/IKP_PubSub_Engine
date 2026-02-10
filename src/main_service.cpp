#include "PubSubEngine.h"
#include "Publisher.h"
#include "Subscriber.h"
#include "Network.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

void printUsage() {
    std::cout << "\n=== PubSub Distributed System ===" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  ./pubsub.exe --engine" << std::endl;
    std::cout << "    Start the PubSub Engine on port 5000" << std::endl;
    std::cout << std::endl;
    std::cout << "  ./pubsub.exe --publisher [--engine-host <host>] [--engine-port <port>]" << std::endl;
    std::cout << "    Start a Publisher (auto-assigned port from 4100+)" << std::endl;
    std::cout << "    Default: localhost:5000" << std::endl;
    std::cout << std::endl;
    std::cout << "  ./pubsub.exe --subscriber --topic <topic1> [--topic <topic2>] ... [--engine-host <host>] [--engine-port <port>]" << std::endl;
    std::cout << "    Start a Subscriber (auto-assigned port from 4200+)" << std::endl;
    std::cout << "    Example: ./pubsub.exe --subscriber --topic \"Analog/MER/220\" --topic \"Status/SWG/1\"" << std::endl;
    std::cout << std::endl;
    std::cout << "In any service, type 'exit' to gracefully shutdown." << std::endl;
    std::cout << "=================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    // Initialize console handler
    ConsoleHandler::init();
    
    std::string mode = argv[1];
    
    // ========================= ENGINE MODE =========================
    if (mode == "--engine") {
        std::cout << "\n=== Starting PubSub Engine ===" << std::endl;
        std::cout << "Listening for publishers and subscribers..." << std::endl;
        std::cout << "Type 'exit' to shutdown." << std::endl;
        
        PubSubEngine engine;
        engine.start();
        
        // Keep running until user types 'exit'
        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        engine.stop();
        std::cout << "Engine shutdown complete." << std::endl;
        return 0;
    }
    
    // ========================= PUBLISHER MODE =========================
    else if (mode == "--publisher") {
        std::string engineHost = "localhost";
        int enginePort = 5000;
        
        // Parse optional engine host/port
        for (int i = 2; i < argc - 1; i++) {
            std::string arg = argv[i];
            if (arg == "--engine-host") {
                engineHost = argv[i + 1];
                i++;
            } else if (arg == "--engine-port") {
                enginePort = std::stoi(argv[i + 1]);
                i++;
            }
        }
        
        std::cout << "\n=== Starting Publisher ===" << std::endl;
        std::cout << "Connecting to engine at " << engineHost << ":" << enginePort << std::endl;
        std::cout << "Publishing messages every 2 seconds..." << std::endl;
        std::cout << "Type 'exit' to shutdown." << std::endl;
        
        Publisher pub(1, engineHost, enginePort);
        pub.start();
        
        // Keep running until user types 'exit'
        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        pub.stop();
        std::cout << "Publisher shutdown complete." << std::endl;
        return 0;
    }
    
    // ========================= SUBSCRIBER MODE =========================
    else if (mode == "--subscriber") {
        std::vector<std::string> topics;
        std::string engineHost = "localhost";
        int enginePort = 5000;
        
        // Parse topics and optional engine host/port
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--topic") {
                if (i + 1 < argc) {
                    topics.push_back(argv[i + 1]);
                    i++;
                }
            } else if (arg == "--engine-host") {
                if (i + 1 < argc) {
                    engineHost = argv[i + 1];
                    i++;
                }
            } else if (arg == "--engine-port") {
                if (i + 1 < argc) {
                    enginePort = std::stoi(argv[i + 1]);
                    i++;
                }
            }
        }
        
        if (topics.empty()) {
            std::cerr << "Error: Subscriber requires at least one topic" << std::endl;
            std::cerr << "Example: ./pubsub.exe --subscriber --topic \"Analog/MER/220\"" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== Starting Subscriber ===" << std::endl;
        std::cout << "Connecting to engine at " << engineHost << ":" << enginePort << std::endl;
        std::cout << "Subscribed to " << topics.size() << " topic(s):" << std::endl;
        for (const auto& topic : topics) {
            std::cout << "  - " << topic << std::endl;
        }
        std::cout << "Type 'exit' to shutdown." << std::endl;
        
        Subscriber sub(1, topics, engineHost, enginePort);
        sub.start();
        
        // Keep running until user types 'exit'
        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        sub.stop();
        std::cout << "Subscriber shutdown complete." << std::endl;
        return 0;
    }
    
    // ========================= INVALID MODE =========================
    else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        printUsage();
        return 1;
    }
}
