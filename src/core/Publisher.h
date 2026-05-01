#ifndef PUBLISHER_H
#define PUBLISHER_H

#include "../Message.h"
#include "../Network.h"
#include "../Serialization.h"
#include <thread>
#include <atomic>
#include <string>
#include <vector>

class Publisher {
private:
    int id;
    int myPort;
    std::string engineHost;
    int enginePort;
    TcpClient engineClient;
    std::thread workerThread;
    std::atomic<bool> running;
    std::vector<std::string> topics;  // Topici koje publisher objavljuje

    void publishLoop();
    
public:
    // Constructor
    // If port is 0 or negative, auto-assign from PortPool
    Publisher(int publisherId,
              const std::string& engine_host = "localhost",
              int engine_port = 5000,
              int port = 0,
              const std::vector<std::string>& topics = {});
    
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