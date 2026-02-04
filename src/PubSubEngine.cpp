#include "PubSubEngine.h"
#include "Subscriber.h"
#include <iostream>
#include <cstring>

PubSubEngine::PubSubEngine() : numTopics(0) {
    topics = new TopicEntry[MAX_TOPICS];
}

PubSubEngine::~PubSubEngine() {
    delete[] topics;
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

void PubSubEngine::subscribe(const char* topic, Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    TopicEntry* entry = getOrCreateTopic(topic);
    if (entry == nullptr) {
        return;
    }
    
    // Provera da li je vec pretplacen
    if (!entry->subscribers.contains(subscriber)) {
        entry->subscribers.pushBack(subscriber);
        std::cout << "[PubSubEngine] Subscriber " << subscriber->getId() 
                  << " pretplacen na topic: " << topic << std::endl;
    } else {
        std::cout << "[PubSubEngine] Subscriber " << subscriber->getId() 
                  << " je vec pretplacen na topic: " << topic << std::endl;
    }
}

void PubSubEngine::unsubscribe(const char* topic, Subscriber* subscriber) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(topic);
    if (index == -1) {
        std::cout << "[PubSubEngine] Topic nije pronadjen: " << topic << std::endl;
        return;
    }
    
    if (topics[index].subscribers.remove(subscriber)) {
        std::cout << "[PubSubEngine] Subscriber " << subscriber->getId() 
                  << " otkazao pretplatu sa topic-a: " << topic << std::endl;
    } else {
        std::cout << "[PubSubEngine] Subscriber " << subscriber->getId() 
                  << " nije bio pretplacen na topic: " << topic << std::endl;
    }
}

void PubSubEngine::publish(const Message& msg) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(msg.topic);
    if (index == -1) {
        std::cout << "[PubSubEngine] Nema subscriber-a za topic: " << msg.topic << std::endl;
        return;
    }
    
    TopicEntry& entry = topics[index];
    
    // Cuvanje poruke u buffer-u
    entry.messageBuffer.push(msg);
    
    // Isporuka svim subscriber-ima
    int deliveredCount = 0;
    for (auto it = entry.subscribers.begin(); it != entry.subscribers.end(); ++it) {
        Subscriber* sub = *it;
        sub->receiveMessage(msg);
        deliveredCount++;
    }
    
    std::cout << "[PubSubEngine] Objavljena poruka na topic-u '" << msg.topic 
              << "' za " << deliveredCount << " subscriber(a)" << std::endl;
}

int PubSubEngine::getSubscriberCount(const char* topic) {
    std::lock_guard<std::mutex> lock(engineMutex);
    
    int index = findTopicIndex(topic);
    if (index == -1) {
        return 0;
    }
    
    return topics[index].subscribers.size();
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
