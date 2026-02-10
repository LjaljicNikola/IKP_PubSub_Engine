#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include "../Message.h"
#include "../DataStructures/CircularBuffer.h"
#include "../Network.h"
#include "../Serialization.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <string>

class Subscriber {
private:
    int id;                                    // Subscriber ID
    int myPort;                                // Assigned port for this subscriber
    std::string engineHost;                    // Engine host
    int enginePort;                            // Engine port
    std::vector<std::string> topics;           // Topics to subscribe to
    
    CircularBuffer<Message> messageQueue;      // Queue for received messages
    TcpClient engineClient;                    // Client to connect to engine
    TcpServer ownServer;                       // Server to receive messages from engine
    
    std::thread processingThread;              // Thread for processing messages
    std::thread receivingThread;               // Thread for receiving messages
    std::mutex queueMutex;                     // Mutex for queue access
    std::condition_variable queueCV;           // Condition variable for queue
    std::atomic<bool> running;                 // Flag to control threads

    int messageCount;
    
    // Worker function that processes messages
    void processMessages();
    
    // Worker function that receives messages from engine
    void receiveLoop();
    
public:
    // Constructor
    // If port is 0 or negative, auto-assign from PortPool
    Subscriber(int subscriberId, const std::vector<std::string>& sub_topics,
               const std::string& engine_host = "localhost", int engine_port = 5000, int port = 0);
    
    // Destructor
    ~Subscriber();
    
    // Start subscriber threads
    void start();
    
    // Stop subscriber threads
    void stop();
    
    // Get subscriber ID
    int getId() const;
};

#endif // SUBSCRIBER_H
