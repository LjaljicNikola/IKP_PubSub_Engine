#ifndef MESSAGE_H
#define MESSAGE_H

#include <ctime>
#include <cstring>

// Enum for message types
enum class MessageType {
    ANALOG = 0,
    STATUS = 1
};

// Enum for topic types
enum class TopicType {
    MER = 0,
    CRB = 1,
    SWG = 2,
    OTHER = 3
};

// Enum for status values
enum class StatusValue {
    OPEN = 0,
    CLOSED = 1,
    SWG_OPEN = 2,
    SWG_CLOSED = 3,
    CRB_OPEN = 4,
    CRB_CLOSED = 5
};

// Union to hold either analog value (float) or status value (enum)
union MessageData {
    float analogValue;
    StatusValue statusValue;
};

// Message structure for Pub/Sub system
struct Message {
    static const int MAX_TOPIC_LEN = 128;
    static const int MAX_HOST_LEN = 64;
    
    char topic[MAX_TOPIC_LEN];              // Topic name (e.g., "Analog/MER/220")
    char publisher_host[MAX_HOST_LEN];      // Publisher host address
    int publisher_port;                     // Publisher port number
    MessageType type;                       // ANALOG or STATUS
    TopicType topicType;                    // MER, CRB, or OTHER
    MessageData data;                       // Holds either float value or StatusValue
    std::time_t timestamp;                  // Message timestamp
    
    // Constructor
    Message() 
        : publisher_port(0), type(MessageType::ANALOG), topicType(TopicType::OTHER), timestamp(std::time(nullptr)) {
        topic[0] = '\0';
        publisher_host[0] = '\0';
        data.analogValue = 0.0f;
    }
    
    // Constructor with topic
    Message(const char* t, MessageType mt, TopicType tt, float val, std::time_t ts = std::time(nullptr))
        : publisher_port(0), type(mt), topicType(tt), timestamp(ts) {
        strncpy(topic, t, MAX_TOPIC_LEN - 1);
        topic[MAX_TOPIC_LEN - 1] = '\0';
        publisher_host[0] = '\0';
        data.analogValue = val;
    }
};

#endif // MESSAGE_H
