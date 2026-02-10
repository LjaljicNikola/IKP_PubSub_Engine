#include "Publisher.h"
#include "../utils/MessageValidator.h"
#include "../utils/MessageFormatter.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>

Publisher::Publisher(int publisherId, const std::string& engine_host, int engine_port, int port)
    : id(publisherId), engineHost(engine_host), enginePort(engine_port), running(false) {
    if (port > 0) {
        myPort = port;  // Use provided port
    } else {
        myPort = PortPool::getNextPublisherPort();  // Auto-assign from pool
    }
}

Publisher::~Publisher() {
    stop();
}

void Publisher::start() {
    if (!running) {
        // Connect to engine
        if (!engineClient.connect(engineHost, enginePort)) {
            std::cerr << "[localhost:" << myPort << "] Greska: Neuspjesna konekcija na engine" << std::endl;
            return;
        }
        
        std::cout << "[localhost:" << myPort << "] Povezan na engine, sluza na portu " << myPort << std::endl;
        
        running = true;
        workerThread = std::thread(&Publisher::publishLoop, this);
    }
}

void Publisher::stop() {
    if (running) {
        running = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }
}



void Publisher::publish(const Message& msg) {
    std::string errorMsg;
    if (!MessageValidator::validate(msg, errorMsg)) {
        std::cout << "[localhost:" << myPort << "] Validacija poruke nije uspela: " << errorMsg << std::endl;
        return;
    }
    
    // Serialize message and add command prefix
    std::vector<uint8_t> message;
    message.push_back(0); // PUBLISH command
    
    std::vector<uint8_t> serialized = Serialization::serialize(msg);
    message.insert(message.end(), serialized.begin(), serialized.end());
    
    if (!engineClient.sendMessage(message)) {
        std::cerr << "[localhost:" << myPort << "] Failed to send message to engine" << std::endl;
        return;
    }
    
    // Use MessageFormatter for consistent message display
    std::cout << "[localhost:" << myPort << "] " << MessageFormatter::formatAsString(msg) << std::endl;
}

void Publisher::publishLoop() {
    std::cout << "[localhost:" << myPort << "] Pokrenut thread za objavljivanje" << std::endl;
    
    int counter = 0;
    while (running && !ConsoleHandler::shouldExit()) {
        // Pauza (10 sekundi)
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        
        if (!running || ConsoleHandler::shouldExit()) break;
        
        // Smenjivanje razlicitih tipova poruka
        Message msg;
        msg.timestamp = std::time(nullptr);
        strncpy(msg.publisher_host, "localhost", Message::MAX_HOST_LEN - 1);
        msg.publisher_port = myPort;
        
        if (counter % 3 == 0) {
            // Objavljivanje analog merenja
            strncpy(msg.topic, "Analog/MER/220", 63);
            msg.type = MessageType::ANALOG;
            msg.topicType = TopicType::MER;
            msg.data.analogValue = 220.5f + (counter % 10) * 0.5f;  // Simulirani napon
        }
        else if (counter % 3 == 1) {
            // Objavljivanje switchgear status-a
            strncpy(msg.topic, "Status/SWG/1", 63);
            msg.type = MessageType::STATUS;
            msg.topicType = TopicType::SWG;
            msg.data.statusValue = (counter % 2 == 0) ? StatusValue::SWG_CLOSED : StatusValue::SWG_OPEN;
        }
        else {
            // Objavljivanje circuit breaker status-a
            strncpy(msg.topic, "Status/CRB/1", 63);
            msg.type = MessageType::STATUS;
            msg.topicType = TopicType::CRB;
            msg.data.statusValue = (counter % 2 == 0) ? StatusValue::CRB_CLOSED : StatusValue::CRB_OPEN;
        }
        
        publish(msg);
        counter++;
    }
    
    std::cout << "[localhost:" << myPort << "] Zaustavljen thread za objavljivanje" << std::endl;
}

int Publisher::getId() const {
    return id;
}
