#ifndef PUBSUB_ENGINE_H
#define PUBSUB_ENGINE_H

#include "../Message.h"
#include "../DataStructures/LinkedList.h"
#include "../DataStructures/CircularBuffer.h"
#include "../Network.h"
#include "../Serialization.h"
#include <mutex>
#include <cstring>
#include <thread>
#include <string>

// Structure to hold subscriber network address
struct SubscriberAddress {
    int port;
    
    SubscriberAddress() : port(0) {}
    explicit SubscriberAddress(int p) : port(p) {}
    
    bool operator==(const SubscriberAddress& other) const {
        return port == other.port;
    }
};

class PubSubEngine {
private:
    // Manual HashMap: topic -> list of subscriber addresses
    static const int MAX_TOPICS = 100;
    
    struct TopicEntry {
        char topic[64];
        LinkedList<SubscriberAddress> subscribers;  // Store subscriber ports
        CircularBuffer<Message> messageBuffer;
        bool occupied;
        
        TopicEntry() : messageBuffer(50), occupied(false) {
            topic[0] = '\0';
        }
    };
    
    TopicEntry* topics;
    int numTopics;
    std::mutex engineMutex;  // Mutex for thread-safe operations
    TcpServer server;        // TCP server for receiving connections
    std::thread acceptThread; // Thread to accept connections
    std::thread validationThread; // Thread for subscriber health checks
    std::atomic<bool> running;
    
    // Hash function for topic names
    int hashTopic(const char* topic) const;
    
    // Find topic entry index
    int findTopicIndex(const char* topic) const;
    
    // Get or create topic entry
    TopicEntry* getOrCreateTopic(const char* topic);
    
    // Accept and handle incoming connections
    void acceptConnections();
    
    // Deliver message to a single subscriber in a separate thread
    void deliverToSubscriber(const SubscriberAddress& addr, const Message& msg, const std::vector<uint8_t>& serialized);
    
    // Validate subscriber health (check if reachable)
    void validateSubscribers();
    
public:
    // Constructor
    PubSubEngine();
    
    // Destructor
    ~PubSubEngine();
    
    // Start the engine server
    void start();
    
    // Stop the engine server
    void stop();
    
    // Internal subscribe method (called by network handler)
    void subscribeInternal(const char* topic, int subscriberPort);
    
    // Internal unsubscribe method
    void unsubscribeInternal(const char* topic, int subscriberPort);
    
    // Publish a message
    void publish(const Message& msg);
    
    // Get number of subscribers for a topic
    int getSubscriberCount(const char* topic);
    
    // Get all topics
    void getAllTopics(char topics[][64], int& count, int maxCount);
};

#endif // PUBSUB_ENGINE_H
