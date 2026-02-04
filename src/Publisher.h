#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "Message.h"
#include <thread>
#include <atomic>

// Forward declaration
class PubSubEngine;

class Publisher {
private:
    int id;                           // Publisher ID
    PubSubEngine* engine;             // Pointer to PubSub Engine
    std::thread workerThread;         // Thread for publishing
    std::atomic<bool> running;        // Flag to control thread
    
    // Worker function that publishes messages
    void publishLoop();
    
    // Validate message before sending
    bool validateMessage(const Message& msg);
    
public:
    // Constructor
    Publisher(int publisherId, PubSubEngine* pubsubEngine);
    
    // Destructor
    ~Publisher();
    
    // Start publisher thread
    void start();
    
    // Stop publisher thread
    void stop();
    
    // Publish a message
    void publish(const Message& msg);
    
    // Get publisher ID
    int getId() const;
};

#endif // PUBLISHER_H
