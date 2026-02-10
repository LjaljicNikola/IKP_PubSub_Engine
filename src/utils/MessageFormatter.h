#ifndef MESSAGE_FORMATTER_H
#define MESSAGE_FORMATTER_H

#include "../Message.h"
#include <string>

class MessageFormatter {
public:
    static std::string formatAsString(const Message& msg);
    static std::string getStatusString(StatusValue status);
};

#endif
