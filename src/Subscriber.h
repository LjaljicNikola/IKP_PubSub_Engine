#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "Message.h"
#include "DataStructures/CircularBuffer.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Subscriber {
private:
    int id;                                    // Subscriber ID
    CircularBuffer<Message> messageQueue;      // Queue for received messages
    std::thread workerThread;                  // Thread for processing messages
    std::mutex queueMutex;                     // Mutex for queue access
    std::condition_variable queueCV;           // Condition variable for queue
    std::atomic<bool> running;                 // Flag to control threadmakemak

    int messageCount;
    
    // Worker function that processes messages
    void processMessages();
    
    // Validate incoming message
    bool validateMessage(const Message& msg);
    
public:
    // Constructor
    explicit Subscriber(int subscriberId);
    
    // Destructor
    ~Subscriber();
    
    // Start subscriber thread
    void start();
    
    // Stop subscriber thread
    void stop();
    
    // Receive message from PubSub Engine
    void receiveMessage(const Message& msg);
    
    // Get subscriber ID
    int getId() const;
};

#endif // SUBSCRIBER_H
