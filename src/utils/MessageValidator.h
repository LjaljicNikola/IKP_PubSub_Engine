#ifndef MESSAGE_VALIDATOR_H
#define MESSAGE_VALIDATOR_H

#include "../Message.h"
#include <string>

class MessageValidator {
public:
    static bool validate(const Message& msg, std::string& errorMsg);
    
private:
    static bool validateAnalogMessage(const Message& msg, std::string& errorMsg);
    static bool validateStatusMessage(const Message& msg, std::string& errorMsg);
};

#endif
