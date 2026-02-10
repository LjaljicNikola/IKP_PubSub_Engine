#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include <string>
#include <vector>

struct CommandLineArgs {
    std::string engineHost = "localhost";
    int enginePort = 5000;
    int port = 0;
};

class CommandLineParser {
public:
    static CommandLineArgs parseCommonArgs(int argc, char* argv[], int startIdx);
    static std::vector<std::string> parseTopics(int argc, char* argv[]);
};

#endif
