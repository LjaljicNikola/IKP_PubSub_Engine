#include "MessageValidator.h"

bool MessageValidator::validate(const Message& msg, std::string& errorMsg) {
    if (msg.topic[0] == '\0') {
        errorMsg = "Topic cannot be empty";
        return false;
    }
    
    if (msg.type == MessageType::ANALOG) {
        return validateAnalogMessage(msg, errorMsg);
    } else if (msg.type == MessageType::STATUS) {
        return validateStatusMessage(msg, errorMsg);
    }
    
    errorMsg = "Unknown message type";
    return false;
}

bool MessageValidator::validateAnalogMessage(const Message& msg, std::string& errorMsg) {
    if (msg.topicType != TopicType::MER) {
        errorMsg = "Analog messages must use MER topic type";
        return false;
    }
    
    if (msg.data.analogValue < -1000.0f || msg.data.analogValue > 10000.0f) {
        errorMsg = "Analog value out of range: -1000.0 to 10000.0";
        return false;
    }
    
    return true;
}

bool MessageValidator::validateStatusMessage(const Message& msg, std::string& errorMsg) {
    if (msg.topicType == TopicType::MER) {
        errorMsg = "Status messages cannot use MER topic type";
        return false;
    }
    
    StatusValue sv = msg.data.statusValue;
    if (sv != StatusValue::SWG_OPEN && sv != StatusValue::SWG_CLOSED &&
        sv != StatusValue::CRB_OPEN && sv != StatusValue::CRB_CLOSED) {
        errorMsg = "Invalid status value";
        return false;
    }
    
    return true;
}
