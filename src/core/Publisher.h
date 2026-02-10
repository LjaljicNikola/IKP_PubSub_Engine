#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "../Message.h"
#include "../Network.h"
#include "../Serialization.h"
#include <thread>
#include <atomic>
#include <string>

class Publisher {
private:
    int id;                           // Publisher ID
    int myPort;                       // Assigned port for this publisher
    std::string engineHost;           // Engine host address
    int enginePort;                   // Engine port
    TcpClient engineClient;           // Client connection to engine
    std::thread workerThread;         // Thread for publishing
    std::atomic<bool> running;        // Flag to control thread
    
    // Worker function that publishes messages
    void publishLoop();
    
public:
    // Constructor
    // If port is 0 or negative, auto-assign from PortPool
    Publisher(int publisherId, const std::string& engine_host = "localhost", int engine_port = 5000, int port = 0);
    
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
