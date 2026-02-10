#include "Subscriber.h"
#include "../utils/MessageValidator.h"
#include "../utils/MessageFormatter.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

static std::mutex coutMutex;

static std::string formatTime(std::time_t ts)
{
    std::stringstream ss;
    std::tm* tm = std::localtime(&ts);
    ss << std::put_time(tm, "%H:%M:%S");
    return ss.str();
}

Subscriber::Subscriber(int subscriberId, const std::vector<std::string>& sub_topics,
                       const std::string& engine_host, int engine_port, int port) 
    : id(subscriberId), engineHost(engine_host), enginePort(engine_port),
      topics(sub_topics), messageQueue(50), running(false), messageCount(0) {
    if (port > 0) {
        myPort = port;  // Use provided port
    } else {
        myPort = PortPool::getNextSubscriberPort();  // Auto-assign from pool
    }
}

Subscriber::~Subscriber() {
    stop();
}

void Subscriber::start() {
    if (!running) {
        // Start own server to receive messages
        if (!ownServer.start(myPort)) {
            std::cerr << "[Subscriber " << id << "] Failed to start server on port " << myPort << std::endl;
            return;
        }
        
        // Give the server a moment to be fully ready for incoming connections
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Connect to engine
        if (!engineClient.connect(engineHost, enginePort)) {
            std::cerr << "[Subscriber " << id << "] Failed to connect to engine at " 
                      << engineHost << ":" << enginePort << std::endl;
            return;
        }
        
        // Subscribe to topics by sending subscription messages to engine
        for (const auto& topic : topics) {
            // Create a simple subscription message format: [command_type(1)] [port(4)] [topic_len(1)] [topic]
            std::vector<uint8_t> subMsg;
            subMsg.push_back(1); // SUBSCRIBE command
            
            // Port in big-endian
            uint32_t port_val = myPort;
            subMsg.push_back((port_val >> 24) & 0xFF);
            subMsg.push_back((port_val >> 16) & 0xFF);
            subMsg.push_back((port_val >> 8) & 0xFF);
            subMsg.push_back(port_val & 0xFF);
            
            // Topic
            subMsg.push_back(topic.length());
            for (char c : topic) {
                subMsg.push_back(c);
            }
            
            if (!engineClient.sendMessage(subMsg)) {
                std::cerr << "[Subscriber " << id << "] Failed to subscribe to topic: " << topic << std::endl;
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id << "] STARTED on port " << myPort << "\n";
        }
        
        running = true;
        receivingThread = std::thread(&Subscriber::receiveLoop, this);
        processingThread = std::thread(&Subscriber::processMessages, this);
    }
}

void Subscriber::stop() {
    if (running) {
        running = false;
        queueCV.notify_all();
        
        // Unsubscribe from all topics
        for (const auto& topic : topics) {
            std::vector<uint8_t> unsubMsg;
            unsubMsg.push_back(2); // UNSUBSCRIBE command
            
            // Topic
            unsubMsg.push_back(topic.length());
            for (char c : topic) {
                unsubMsg.push_back(c);
            }
            
            engineClient.sendMessage(unsubMsg);
        }
        
        engineClient.disconnect();
        ownServer.stop();

        if (receivingThread.joinable()) {
            receivingThread.join();
        }
        
        if (processingThread.joinable()) {
            processingThread.join();
        }

        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "[Subscriber " << id << "] STOPPED | ukupno poruka: "
                  << messageCount << "\n";
    }
}

void Subscriber::receiveLoop() {
    while (running && !ConsoleHandler::shouldExit()) {
        std::vector<uint8_t> serialized = ownServer.receiveMessage();
        
        if (serialized.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        
        // Deserialize message
        Message msg = Serialization::deserialize(serialized.data(), serialized.size());
        
        // Push to queue
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            messageQueue.push(msg);
        }
        queueCV.notify_one();
    }
}

void Subscriber::processMessages() {

    while (running && !ConsoleHandler::shouldExit()) {

        Message msg;
        bool hasMessage = false;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait_for(lock,
                std::chrono::milliseconds(100),
                [this] {
                    return !messageQueue.isEmpty() || !running;
                });

            if (!running || ConsoleHandler::shouldExit()) break;

            hasMessage = messageQueue.pop(msg);
        }

        if (!hasMessage)
            continue;

        std::string errorMsg;
        if (!MessageValidator::validate(msg, errorMsg)) {

            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id
                      << "] VALIDACIJA NIJE USPESNA: " << errorMsg << "\n";
            continue;
        }

        // Check if message topic is in subscribed topics
        bool topicMatch = false;
        for (const auto& topic : topics) {
            if (topic == msg.topic) {
                topicMatch = true;
                break;
            }
        }
        
        if (!topicMatch) {
            // Skip messages not matching subscribed topics
            continue;
        }

        messageCount++;

        std::lock_guard<std::mutex> lock(coutMutex);

        std::cout << "\n--------------------------------------\n";
        std::cout << "PUBLISHER: " << msg.publisher_host
                  << ":" << msg.publisher_port
                  << " | PORUKA #" << messageCount
                  << " | " << formatTime(msg.timestamp) << "\n";

        std::cout << "Topic: " << msg.topic << "\n";

        if (msg.type == MessageType::ANALOG) {

            std::cout << "Tip: ANALOG\n";
            std::cout << "Vrednost: "
                      << msg.data.analogValue << "\n";
        }
        else {

            const char* statusStr =
                (msg.data.statusValue == StatusValue::SWG_OPEN ||
                 msg.data.statusValue == StatusValue::CRB_OPEN)
                ? "OPEN" : "CLOSED";

            std::cout << "Tip: STATUS\n";
            std::cout << "Stanje: " << statusStr << "\n";
        }

        std::cout << "--------------------------------------\n";
    }
}

int Subscriber::getId() const {
    return id;
}
