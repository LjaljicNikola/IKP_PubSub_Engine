#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstring>

// Enum for message types
enum class MessageType {
    ANALOG,    // For measurements (MER)
    STATUS     // For switches (SWG) and circuit breakers (CRB)
};

// Enum for status values
enum class StatusValue {
    SWG_OPEN = 0,      // Switchgear open
    SWG_CLOSED = 1,    // Switchgear closed
    CRB_OPEN = 0,      // Circuit breaker open
    CRB_CLOSED = 1     // Circuit breaker closed
};

// Enum for topic types
enum class TopicType {
    MER,    // Measurement (analog)
    SWG,    // Switchgear (status)
    CRB     // Circuit breaker (status)
};

// Structure to hold message data
struct Message {
    char topic[64];           // Topic name (e.g., "Analog/MER/220", "Status/SWG/1")
    MessageType type;         // Message type (analog or status)
    TopicType topicType;      // Topic type (MER, SWG, CRB)
    
    union {
        float analogValue;    // For analog messages
        StatusValue statusValue;  // For status messages
    } data;
    
    long timestamp;           // Message timestamp
    
    // Constructor
    Message() : type(MessageType::ANALOG), topicType(TopicType::MER), timestamp(0) {
        topic[0] = '\0';
        data.analogValue = 0.0f;
    }
    
    // Copy constructor
    Message(const Message& other) {
        strncpy(topic, other.topic, 63);
        topic[63] = '\0';
        type = other.type;
        topicType = other.topicType;
        timestamp = other.timestamp;
        
        if (type == MessageType::ANALOG) {
            data.analogValue = other.data.analogValue;
        } else {
            data.statusValue = other.data.statusValue;
        }
    }
    
    // Assignment operator
    Message& operator=(const Message& other) {
        if (this != &other) {
            strncpy(topic, other.topic, 63);
            topic[63] = '\0';
            type = other.type;
            topicType = other.topicType;
            timestamp = other.timestamp;
            
            if (type == MessageType::ANALOG) {
                data.analogValue = other.data.analogValue;
            } else {
                data.statusValue = other.data.statusValue;
            }
        }
        return *this;
    }
};

#endif // MESSAGE_H
