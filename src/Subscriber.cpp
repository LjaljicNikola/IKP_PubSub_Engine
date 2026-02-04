#include "Subscriber.h"
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

Subscriber::Subscriber(int subscriberId) 
    : id(subscriberId), messageQueue(50), running(false), messageCount(0) {
}

Subscriber::~Subscriber() {
    stop();
}

void Subscriber::start() {
    if (!running) {
        running = true;
        workerThread = std::thread(&Subscriber::processMessages, this);

        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "[Subscriber " << id << "] STARTED\n";
    }
}

void Subscriber::stop() {
    if (running) {
        running = false;
        queueCV.notify_all();

        if (workerThread.joinable()) {
            workerThread.join();
        }

        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "[Subscriber " << id << "] STOPPED | ukupno poruka: "
                  << messageCount << "\n";
    }
}

void Subscriber::receiveMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(queueMutex);
    messageQueue.push(msg);
    queueCV.notify_one();
}

bool Subscriber::validateMessage(const Message& msg) {

    if (msg.type == MessageType::ANALOG) {

        if (msg.topicType != TopicType::MER) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id 
                      << "] GRESKA: Analog mora biti MER\n";
            return false;
        }

        if (msg.data.analogValue < -1000.0f ||
            msg.data.analogValue > 10000.0f) {

            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id
                      << "] UPOZORENJE: analog van opsega -> "
                      << msg.data.analogValue << "\n";
        }

        return true;
    } 
    else if (msg.type == MessageType::STATUS) {

        if (msg.topicType == TopicType::MER) {
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id
                      << "] GRESKA: Status ne moze biti MER\n";
            return false;
        }

        if(msg.data.statusValue != StatusValue::SWG_OPEN &&
           msg.data.statusValue != StatusValue::SWG_CLOSED &&
           msg.data.statusValue != StatusValue::CRB_OPEN &&
           msg.data.statusValue != StatusValue::CRB_CLOSED) {

            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout<<"[Subscriber "<<id
                     <<"] GRESKA: Nevalidna status vrednost\n";
            return false;
        }

        return true;
    }

    return false;
}

void Subscriber::processMessages() {

    while (running) {

        Message msg;
        bool hasMessage = false;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait_for(lock,
                std::chrono::milliseconds(100),
                [this] {
                    return !messageQueue.isEmpty() || !running;
                });

            if (!running) break;

            hasMessage = messageQueue.pop(msg);
        }

        if (!hasMessage)
            continue;

        if (!validateMessage(msg)) {

            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "[Subscriber " << id
                      << "] VALIDACIJA NIJE USPESNA\n";
            continue;
        }

        messageCount++;

        std::lock_guard<std::mutex> lock(coutMutex);

        std::cout << "\n--------------------------------------\n";
        std::cout << "SUBSCRIBER " << id
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
