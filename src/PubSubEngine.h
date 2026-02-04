#ifndef PUBSUB_ENGINE_H
#define PUBSUB_ENGINE_H

#include "Message.h"
#include "DataStructures/LinkedList.h"
#include "DataStructures/CircularBuffer.h"
#include <mutex>
#include <cstring>

// Forward declaration
class Subscriber;

// Structure to hold topic name as a string key for HashMap
struct TopicKey {
    char name[64];
    
    TopicKey() {
        name[0] = '\0';
    }
    
    TopicKey(const char* topicName) {
        strncpy(name, topicName, 63);
        name[63] = '\0';
    }
    
    TopicKey(const TopicKey& other) {
        strncpy(name, other.name, 63);
        name[63] = '\0';
    }
    
    TopicKey& operator=(const TopicKey& other) {
        if (this != &other) {
            strncpy(name, other.name, 63);
            name[63] = '\0';
        }
        return *this;
    }
    
    bool operator==(const TopicKey& other) const {
        return strcmp(name, other.name) == 0;
    }
};

class PubSubEngine {
private:
    // Manual HashMap: topic -> list of subscribers
    // We'll use a simple array-based approach with linear probing
    static const int MAX_TOPICS = 100;
    
    struct TopicEntry {
        char topic[64];
        LinkedList<Subscriber*> subscribers;
        CircularBuffer<Message> messageBuffer;
        bool occupied;
        
        TopicEntry() : messageBuffer(50), occupied(false) {
            topic[0] = '\0';
        }
    };
    
    TopicEntry* topics;
    int numTopics;
    std::mutex engineMutex;  // Mutex for thread-safe operations
    
    // Hash function for topic names
    int hashTopic(const char* topic) const;
    
    // Find topic entry index
    int findTopicIndex(const char* topic) const;
    
    // Get or create topic entry
    TopicEntry* getOrCreateTopic(const char* topic);
    
public:
    // Constructor
    PubSubEngine();
    
    // Destructor
    ~PubSubEngine();
    
    // Subscribe to a topic
    void subscribe(const char* topic, Subscriber* subscriber);
    
    // Unsubscribe from a topic
    void unsubscribe(const char* topic, Subscriber* subscriber);
    
    // Publish a message
    void publish(const Message& msg);
    
    // Get number of subscribers for a topic
    int getSubscriberCount(const char* topic);
    
    // Get all topics
    void getAllTopics(char topics[][64], int& count, int maxCount);
};

#endif // PUBSUB_ENGINE_H
