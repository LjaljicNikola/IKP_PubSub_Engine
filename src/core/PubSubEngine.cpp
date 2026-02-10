#include "PubSubEngine.h"
#include <iostream>
#include <cstring>
#include <chrono>

PubSubEngine::PubSubEngine() : numTopics(0), running(false) {
    topics = new TopicEntry[MAX_TOPICS];
}

PubSubEngine::~PubSubEngine() {
    stop();
    delete[] topics;
}

void PubSubEngine::start() {
    if (!running) {
        running = true;
        
        int enginePort = PortPool::getEnginePort();
        if (!server.start(enginePort)) {
            std::cerr << "[PubSubEngine] Failed to start server on port " << enginePort << std::endl;
            running = false;
            return;
        }
        
        std::cout << "[PubSubEngine] Engine started on port " << enginePort << std::endl;
        
        acceptThread = std::thread(&PubSubEngine::acceptConnections, this);
        acceptThread.detach();
        
        // Start validation thread for subscriber health checks
        validationThread = std::thread(&PubSubEngine::validateSubscribers, this);
        validationThread.detach();
        std::cout << "[PubSubEngine] Subscriber validation thread started" << std::endl;
    }
}

void PubSubEngine::stop() {
    if (running) {
        running = false;
        server.stop();
        std::cout << "[PubSubEngine] Engine stopped" << std::endl;
    }
}

void PubSubEngine::acceptConnections() {
    while (running && !ConsoleHandler::shouldExit()) {
        std::vector<uint8_t> data = server.receiveMessage();
        
        if (data.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        // Parse command: [command(1)] [data...]
        if (data.empty()) continue;
        
        uint8_t cmd = data[0];
        
        if (cmd == 0) {
            // PUBLISH command: [data...]
            // The rest is a serialized Message
            Message msg = Serialization::deserialize(data.data() + 1, data.size() - 1);
            publish(msg);
        } else if (cmd == 1) {
            // SUBSCRIBE command: [port(4)] [topic_len(1)] [topic...]
            if (data.size() < 6) continue;
            
            uint32_t port_val = ((uint32_t)data[1] << 24) |
                                ((uint32_t)data[2] << 16) |
                                ((uint32_t)data[3] << 8) |
                                (uint32_t)data[4];
            
            uint8_t topic_len = data[5];
            if (data.size() < static_cast<size_t>(6 + topic_len)) continue;
            
            char topic[65];
            memcpy(topic, &data[6], topic_len);
            topic[topic_len] = '\0';
            
            subscribeInternal(topic, port_val);
        } else if (cmd == 2) {
            // UNSUBSCRIBE command: [topic_len(1)] [topic...]
            if (data.size() < 2) continue;
            
            uint8_t topic_len = data[1];
            if (data.size() < static_cast<size_t>(2 + topic_len)) continue;
            
            char topic[65];
            memcpy(topic, &data[2], topic_len);
            topic[topic_len] = '\0';
            
            unsubscribeInternal(topic, 0);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int PubSubEngine::hashTopic(const char* topic) const {
    unsigned long hash = 5381;
    int c;
    const char* str = topic;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % MAX_TOPICS;
}

int PubSubEngine::findTopicIndex(const char* topic) const {
    int index = hashTopic(topic);
    int originalIndex = index;
    
    while (topics[index].occupied) {
        if (strcmp(topics[index].topic, topic) == 0) {
            return index;
        }
        index = (index + 1) % MAX_TOPICS;
        if (index == originalIndex) {
            break;  // Pun krug, nije pronadjen
        }
    }
    
    return -1;  // Nije pronadjen
}

PubSubEngine::TopicEntry* PubSubEngine::getOrCreateTopic(const char* topic) {
    int index = findTopicIndex(topic);
    
    if (index != -1) {
        return &topics[index];
    }
    
    // Kreiranje novog topic-a
    if (numTopics >= MAX_TOPICS) {
        std::cout << "[PubSubEngine] GRESKA: Dostignut maksimalan broj topic-a!" << std::endl;
        return nullptr;
    }
    
    index = hashTopic(topic);
    int originalIndex = index;
    
    // Linear probing da pronadje prazan slot
    while (topics[index].occupied) {
        index = (index + 1) % MAX_TOPICS;
        if (index == originalIndex) {
            std::cout << "[PubSubEngine] GRESKA: Tabela topic-a je puna!" << std::endl;
            return nullptr;
        }
    }
    
    // Inicijalizacija novog topic-a
    strncpy(topics[index].topic, topic, 63);
    topics[index].topic[63] = '\0';
    topics[index].occupied = true;
    numTopics++;
    
    std::cout << "[PubSubEngine] Kreiran novi topic: " << topic << std::endl;
    
    return &topics[index];
}

void PubSubEngine::subscribeInternal(const char* topic, int subscriberPort) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    TopicEntry* entry = getOrCreateTopic(topic);
    if (entry == nullptr) {
        return;
    }
    
    // Check if already subscribed
    SubscriberAddress addr(subscriberPort);
    if (!entry->subscribers.contains(addr)) {
        entry->subscribers.pushBack(addr);
        std::cout << "[PubSubEngine] Subscriber on port " << subscriberPort 
                  << " subscribed to topic: " << topic << std::endl;
    } else {
        std::cout << "[PubSubEngine] Subscriber on port " << subscriberPort 
                  << " already subscribed to topic: " << topic << std::endl;
    }
}

void PubSubEngine::unsubscribeInternal(const char* topic, int subscriberPort) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(topic);
    if (index == -1) {
        std::cout << "[PubSubEngine] Topic not found: " << topic << std::endl;
        return;
    }
    
    SubscriberAddress addr(subscriberPort);
    if (topics[index].subscribers.remove(addr)) {
        std::cout << "[PubSubEngine] Subscriber on port " << subscriberPort 
                  << " unsubscribed from topic: " << topic << std::endl;
    } else {
        std::cout << "[PubSubEngine] Subscriber on port " << subscriberPort 
                  << " was not subscribed to topic: " << topic << std::endl;
    }
}

void PubSubEngine::deliverToSubscriber(const SubscriberAddress& addr, const Message& msg, const std::vector<uint8_t>& serialized) {
    // Attempt delivery to a single subscriber in its own thread
    TcpClient client;
    if (client.connect("localhost", addr.port)) {
        if (client.sendMessage(serialized)) {
            std::cout << "[PubSubEngine:DELIVERY] Message published to topic '" << msg.topic 
                      << "' -> Subscriber on port " << addr.port << " [SUCCESS]" << std::endl;
        } else {
            std::cerr << "[PubSubEngine:DELIVERY] Failed to send to port " << addr.port << std::endl;
        }
        client.disconnect();
    } else {
        std::cerr << "[PubSubEngine:DELIVERY] Failed to connect to port " << addr.port << std::endl;
    }
}

void PubSubEngine::publish(const Message& msg) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(msg.topic);
    if (index == -1) {
        std::cout << "[PubSubEngine] Nema pretplatnika za topic: " << msg.topic << std::endl;
        return;
    }
    
    TopicEntry& entry = topics[index];
    
    // Save message to buffer
    entry.messageBuffer.push(msg);
    std::cout << "[PubSubEngine] Message published to topic '" << msg.topic << "'" << std::endl;
    
    // Serialize message once
    std::vector<uint8_t> serialized = Serialization::serialize(msg);
    
    // Get subscriber list snapshot (to avoid holding lock during delivery)
    std::vector<SubscriberAddress> subscribersCopy;
    for (auto it = entry.subscribers.begin(); it != entry.subscribers.end(); ++it) {
        subscribersCopy.push_back(*it);
    }
    
    int totalSubscribers = subscribersCopy.size();
    std::cout << "[PubSubEngine] Delivering to " << totalSubscribers << " subscriber(s)..." << std::endl;
    
    // Release lock before spawning delivery threads
    lock.~lock_guard();
    
    // Spawn a thread for each subscriber to deliver in parallel
    std::vector<std::thread> deliveryThreads;
    for (const auto& addr : subscribersCopy) {
        deliveryThreads.emplace_back(&PubSubEngine::deliverToSubscriber, this, addr, msg, serialized);
    }
    
    // Detach threads to allow publish() to return immediately
    for (auto& t : deliveryThreads) {
        if (t.joinable()) {
            t.detach();
        }
    }
}

int PubSubEngine::getSubscriberCount(const char* topic) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(topic);
    if (index == -1) {
        return 0;
    }
    
    return topics[index].subscribers.size();
}

void PubSubEngine::validateSubscribers() {
    // Periodically validate that all registered subscribers are reachable
    while (running && !ConsoleHandler::shouldExit()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::lock_guard<std::mutex> lock(engineMutex);
        
        // Check each topic's subscribers
        for (int i = 0; i < MAX_TOPICS; i++) {
            if (!topics[i].occupied) continue;
            
            std::vector<SubscriberAddress> deadSubscribers;
            
            // Test connection to each subscriber
            for (auto it = topics[i].subscribers.begin(); it != topics[i].subscribers.end(); ++it) {
                TcpClient testClient;
                if (!testClient.connect("localhost", it->port)) {
                    // Subscriber is unreachable
                    deadSubscribers.push_back(*it);
                    std::cout << "[PubSubEngine:VALIDATION] Subscriber on port " << it->port 
                              << " is unreachable for topic '" << topics[i].topic << "'" << std::endl;
                } else {
                    testClient.disconnect();
                }
            }
            
            // Remove dead subscribers
            for (const auto& dead : deadSubscribers) {
                topics[i].subscribers.remove(dead);
                std::cout << "[PubSubEngine:VALIDATION] Removed unreachable subscriber on port " 
                          << dead.port << " from topic '" << topics[i].topic << "'" << std::endl;
            }
        }
    }
}

void PubSubEngine::getAllTopics(char topicList[][64], int& count, int maxCount) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    count = 0;
    for (int i = 0; i < MAX_TOPICS && count < maxCount; i++) {
        if (topics[i].occupied) {
            strncpy(topicList[count], topics[i].topic, 63);
            topicList[count][63] = '\0';
            count++;
        }
    }
}
