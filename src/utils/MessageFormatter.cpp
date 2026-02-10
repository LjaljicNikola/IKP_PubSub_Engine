#include "MessageFormatter.h"
#include <sstream>
#include <iomanip>

std::string MessageFormatter::formatAsString(const Message& msg) {
    std::ostringstream oss;
    oss << "Topic: " << msg.topic << " | ";
    
    if (msg.type == MessageType::ANALOG) {
        oss << "Type: ANALOG | Value: " << std::fixed << std::setprecision(2) 
            << msg.data.analogValue;
    } else if (msg.type == MessageType::STATUS) {
        oss << "Type: STATUS | Value: " << getStatusString(msg.data.statusValue);
    }
    
    return oss.str();
}

std::string MessageFormatter::getStatusString(StatusValue status) {
    return (status == StatusValue::SWG_OPEN || status == StatusValue::CRB_OPEN) 
        ? "OPEN" 
        : "CLOSED";
}
