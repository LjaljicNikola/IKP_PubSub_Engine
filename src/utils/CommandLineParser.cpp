#include "CommandLineParser.h"

CommandLineArgs CommandLineParser::parseCommonArgs(int argc, char* argv[], int startIdx) {
    CommandLineArgs args;
    
    for (int i = startIdx; i < argc - 1; i++) {
        std::string arg = argv[i];
        
        if (arg == "--engine-host") {
            args.engineHost = argv[i + 1];
            i++;
        } else if (arg == "--engine-port") {
            args.enginePort = std::stoi(argv[i + 1]);
            i++;
        } else if (arg == "--port") {
            args.port = std::stoi(argv[i + 1]);
            i++;
        }
    }
    
    return args;
}

std::vector<std::string> CommandLineParser::parseTopics(int argc, char* argv[]) {
    std::vector<std::string> topics;
    
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--topic") {
            if (i + 1 < argc) {
                topics.push_back(argv[i + 1]);
                i++;
            }
        }
    }
    
    return topics;
}
