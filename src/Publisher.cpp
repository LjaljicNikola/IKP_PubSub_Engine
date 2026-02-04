#include "Publisher.h"
#include "PubSubEngine.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>

Publisher::Publisher(int publisherId, PubSubEngine* pubsubEngine)
    : id(publisherId), engine(pubsubEngine), running(false) {
}

Publisher::~Publisher() {
    stop();
}

void Publisher::start() {
    if (!running) {
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

bool Publisher::validateMessage(const Message& msg) {
    // Provera da topic nije prazan
    if (strlen(msg.topic) == 0) {
        std::cout << "[Publisher " << id << "] GRESKA: Topic ne moze biti prazan!" << std::endl;
        return false;
    }
    
    // Validacija na osnovu tipa poruke
    if (msg.type == MessageType::ANALOG) {
        // Analog poruke moraju biti MER tip
        if (msg.topicType != TopicType::MER) {
            std::cout << "[Publisher " << id << "] GRESKA: Analog poruke moraju koristiti MER topic tip!" << std::endl;
            return false;
        }
        // Provera validnog opsega analog vrednosti (primer)
        if (msg.data.analogValue < -1000.0f || msg.data.analogValue > 10000.0f) {
            std::cout << "[Publisher " << id << "] UPOZORENJE: Analog vrednost mozda nije u validnom opsegu: " 
                      << msg.data.analogValue << std::endl;
        }
    }
    else if (msg.type == MessageType::STATUS) {
        // Status poruke moraju biti SWG ili CRB tip
        if (msg.topicType == TopicType::MER) {
            std::cout << "[Publisher " << id << "] GRESKA: Status poruke ne mogu koristiti MER topic tip!" << std::endl;
            return false;
        }
    }
    else {
        std::cout << "[Publisher " << id << "] GRESKA: Nepoznat tip poruke!" << std::endl;
        return false;
    }
    
    return true;
}

void Publisher::publish(const Message& msg) {
    if (!validateMessage(msg)) {
        std::cout << "[Publisher " << id << "] Validacija poruke nije uspela, poruka se ne objavljuje!" << std::endl;
        return;
    }
    
    // Slanje u PubSub Engine
    engine->publish(msg);
    
    // Logovanje
    if (msg.type == MessageType::ANALOG) {
        std::cout << "[Publisher " << id << "] Objavljen ANALOG na '" << msg.topic 
                  << "': " << msg.data.analogValue << std::endl;
    } else {
        const char* statusStr = (msg.data.statusValue == StatusValue::SWG_OPEN || 
                                msg.data.statusValue == StatusValue::CRB_OPEN) 
                                ? "OPEN" : "CLOSED";
        std::cout << "[Publisher " << id << "] Objavljen STATUS na '" << msg.topic 
                  << "': " << statusStr << std::endl;
    }
}

void Publisher::publishLoop() {
    std::cout << "[Publisher " << id << "] Pokrenut thread za objavljivanje" << std::endl;
    
    int counter = 0;
    while (running) {
        // Pauza
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        if (!running) break;
        
        // Smenjivanje razlicitih tipova poruka
        Message msg;
        msg.timestamp = std::time(nullptr);
        
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
    
    std::cout << "[Publisher " << id << "] Zaustavljen thread za objavljivanje" << std::endl;
}

int Publisher::getId() const {
    return id;
}
