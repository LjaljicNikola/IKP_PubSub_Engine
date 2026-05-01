#include "core/PubSubEngine.h"
#include "core/Publisher.h"
#include "core/Subscriber.h"
#include "Network.h"
#include "monitor/HttpServer.h"
#include "utils/CommandLineParser.h"
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
    std::cout << "  ./pubsub.exe --publisher [--port <port>] [--engine-host <host>] [--engine-port <port>]" << std::endl;
    std::cout << "    Start a Publisher (auto-assigned if --port not specified)" << std::endl;
    std::cout << "    Default: localhost:5000" << std::endl;
    std::cout << "    Example: ./pubsub.exe --publisher --port 4101 --engine-host localhost --engine-port 5000" << std::endl;
    std::cout << std::endl;
    std::cout << "  ./pubsub.exe --subscriber --topic <topic1> [--topic <topic2>] ... [--port <port>] [--engine-host <host>] [--engine-port <port>]" << std::endl;
    std::cout << "    Start a Subscriber (auto-assigned if --port not specified)" << std::endl;
    std::cout << "    Example: ./pubsub.exe --subscriber --topic \"Analog/MER/220\" --port 4201 --engine-host localhost --engine-port 5000" << std::endl;
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

        // Pokreni HTTP monitoring dashboard na portu 8080
        HttpServer httpServer(&engine, 8080);
        httpServer.start();
        
        // Keep running until user types 'exit'
        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        engine.stop();
        httpServer.stop();
        std::cout << "Engine shutdown complete." << std::endl;
        std::cout << "\nMain thread exiting..." << std::endl;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 0;
    }
    
    // ========================= PUBLISHER MODE =========================
    else if (mode == "--publisher") {
        auto args      = CommandLineParser::parseCommonArgs(argc, argv, 2);
        auto pubTopics = CommandLineParser::parseTopics(argc, argv);

        std::cout << "\n=== Starting Publisher ===" << std::endl;
        std::cout << "Connecting to engine at " << args.engineHost << ":" << args.enginePort << std::endl;
        if (pubTopics.empty()) {
            std::cout << "Topics: Analog/MER/220, Status/SWG/1, Status/CRB/1 (podrazumevano)" << std::endl;
        } else {
            std::cout << "Topics: ";
            for (const auto& t : pubTopics) std::cout << t << " ";
            std::cout << std::endl;
        }
        std::cout << "Type 'exit' to shutdown." << std::endl;

        Publisher pub(1, args.engineHost, args.enginePort, args.port, pubTopics);
        pub.start();

        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        pub.stop();
        std::cout << "Publisher shutdown complete." << std::endl;
        std::cout << "\nMain thread exiting..." << std::endl;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 0;
    }
    
    // ========================= SUBSCRIBER MODE =========================
    else if (mode == "--subscriber") {
        auto topics = CommandLineParser::parseTopics(argc, argv);
        auto args = CommandLineParser::parseCommonArgs(argc, argv, 2);
        
        if (topics.empty()) {
            std::cerr << "Error: Subscriber requires at least one topic" << std::endl;
            std::cerr << "Example: ./pubsub.exe --subscriber --topic \"Analog/MER/220\"" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== Starting Subscriber ===" << std::endl;
        std::cout << "Connecting to engine at " << args.engineHost << ":" << args.enginePort << std::endl;
        std::cout << "Subscribed to " << topics.size() << " topic(s):" << std::endl;
        for (const auto& topic : topics) {
            std::cout << "  - " << topic << std::endl;
        }
        std::cout << "Type 'exit' to shutdown." << std::endl;
        
        Subscriber sub(1, topics, args.engineHost, args.enginePort, args.port);
        sub.start();
        
        while (!ConsoleHandler::shouldExit()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        sub.stop();
        std::cout << "Subscriber shutdown complete." << std::endl;
        std::cout << "\nMain thread exiting..." << std::endl;
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 0;
    }
    
    // ========================= INVALID MODE =========================
    else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        printUsage();
        return 1;
    }
}