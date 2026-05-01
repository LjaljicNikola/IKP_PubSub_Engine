#include "Publisher.h"
#include "../utils/MessageValidator.h"
#include "../utils/MessageFormatter.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <map>

Publisher::Publisher(int publisherId, const std::string& engine_host, int engine_port, int port, const std::vector<std::string>& topicList)
    : id(publisherId), engineHost(engine_host), enginePort(engine_port), running(false), topics(topicList) {
    if (port > 0) {
        myPort = port;
    } else {
        myPort = PortPool::getNextPublisherPort();
    }

    // Ako nisu zadati topici, koristi podrazumevane
    if (topics.empty()) {
        topics = { "Analog/MER/220", "Status/SWG/1", "Status/CRB/1" };
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
    std::cout << "[localhost:" << myPort << "] Topici: ";
    for (const auto& t : topics) std::cout << t << " ";
    std::cout << std::endl;

    // Inicijalizuj pocetne vrednosti po topiku (izvuci baznu vrednost iz naziva)
    std::map<std::string, float> currentValues;
    std::srand((unsigned)std::time(nullptr) + myPort);

    for (const auto& t : topics) {
        float base = 220.0f;
        // Izvuci broj iz kraja naziva topica (npr. "Analog/MER/300" -> 300)
        size_t lastSlash = t.rfind('/');
        if (lastSlash != std::string::npos) {
            try { base = std::stof(t.substr(lastSlash + 1)); } catch (...) {}
        }
        currentValues[t] = base;
    }

    int counter = 0;
    while (running && !ConsoleHandler::shouldExit()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        if (!running || ConsoleHandler::shouldExit()) break;

        const std::string& topicName = topics[counter % topics.size()];

        Message msg;
        msg.timestamp = std::time(nullptr);
        strncpy(msg.publisher_host, "localhost", Message::MAX_HOST_LEN - 1);
        msg.publisher_port = myPort;
        strncpy(msg.topic, topicName.c_str(), 63);

        if (topicName.find("SWG") != std::string::npos) {
            msg.type             = MessageType::STATUS;
            msg.topicType        = TopicType::SWG;
            msg.data.statusValue = (counter % 2 == 0) ? StatusValue::SWG_CLOSED : StatusValue::SWG_OPEN;
        }
        else if (topicName.find("CRB") != std::string::npos) {
            msg.type             = MessageType::STATUS;
            msg.topicType        = TopicType::CRB;
            msg.data.statusValue = (counter % 2 == 0) ? StatusValue::CRB_CLOSED : StatusValue::CRB_OPEN;
        }
        else {
            // Bazna vrednost iz naziva topica
            float base = currentValues[topicName];
            size_t lastSlash = topicName.rfind('/');
            float baseNominal = 220.0f;
            if (lastSlash != std::string::npos) {
                try { baseNominal = std::stof(topicName.substr(lastSlash + 1)); } catch (...) {}
            }

            // 10% sansa za spike van granica +-10
            bool spike = (std::rand() % 100) < 10;

            if (spike) {
                // Spike: +-12 do +-18 od nominalne vrednosti
                float spikeAmt = 12.0f + (std::rand() % 7);
                currentValues[topicName] = baseNominal + ((std::rand() % 2 == 0) ? spikeAmt : -spikeAmt);
            } else {
                // Normalan random walk: mali korak +-0.5, driftuje ka nominalnoj
                float step   = ((std::rand() % 100) - 50) / 100.0f;  // -0.5 do +0.5
                float drift  = (baseNominal - currentValues[topicName]) * 0.05f; // vraca ka nominalnoj
                currentValues[topicName] += step + drift;
                // Ograni na +-10 u normalnom rezimu
                float maxDev = 10.0f;
                if (currentValues[topicName] > baseNominal + maxDev)
                    currentValues[topicName] = baseNominal + maxDev;
                if (currentValues[topicName] < baseNominal - maxDev)
                    currentValues[topicName] = baseNominal - maxDev;
            }

            // Zaokruzi na 2 decimale
            currentValues[topicName] = std::round(currentValues[topicName] * 100.0f) / 100.0f;

            msg.type             = MessageType::ANALOG;
            msg.topicType        = TopicType::MER;
            msg.data.analogValue = currentValues[topicName];
        }

        publish(msg);
        counter++;
    }

    std::cout << "[localhost:" << myPort << "] Zaustavljen thread za objavljivanje" << std::endl;
}

int Publisher::getId() const {
    return id;
}