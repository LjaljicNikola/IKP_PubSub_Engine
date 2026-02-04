/*
originalan main

#include "PubSubEngine.h"
#include "Publisher.h"
#include "Subscriber.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

void printMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   PubSub System - Industrijski Protokoli" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Pokreni automatski demo (publishers + subscribers)" << std::endl;
    std::cout << "2. Posalji poruku rucno" << std::endl;
    std::cout << "3. Prikazi sve topic-e" << std::endl;
    std::cout << "4. Prikazi broj subscriber-a po topic-u" << std::endl;
    std::cout << "5. Izlaz" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Unesite izbor: ";
}

void printTopicMenu() {
    std::cout << "\nIzaberite tip poruke:" << std::endl;
    std::cout << "1. Analog (MER - Measurement)" << std::endl;
    std::cout << "2. Status (SWG - Switchgear)" << std::endl;
    std::cout << "3. Status (CRB - Circuit Breaker)" << std::endl;
    std::cout << "Unesite izbor: ";
}

int main() {
    std::cout << "=== PubSub Engine Demo ===" << std::endl;
    std::cout << "FTN - Projekat iz Industrijskih Komunikacionih Protokola" << std::endl;
    std::cout << "\nOvaj demo implementira Publisher-Subscriber sistem sa:" << std::endl;
    std::cout << "- Rucno implementiranim strukturama podataka (CircularBuffer, LinkedList, HashMap)" << std::endl;
    std::cout << "- Thread-safe operacijama" << std::endl;
    std::cout << "- Validacijom poruka" << std::endl;
    std::cout << "- Vise tipova topic-a (MER, SWG, CRB)" << std::endl;
    std::cout << "\nPritisnite Enter za nastavak...";
    std::cin.get();
    
    // Kreiranje PubSub Engine-a
    PubSubEngine engine;
    
    // Kreiranje subscriber-a
    Subscriber* sub1 = new Subscriber(1);
    Subscriber* sub2 = new Subscriber(2);
    Subscriber* sub3 = new Subscriber(3);
    
    // Pretplata na topic-e
    engine.subscribe("Analog/MER/220", sub1);
    engine.subscribe("Analog/MER/220", sub2);
    engine.subscribe("Status/SWG/1", sub2);
    engine.subscribe("Status/SWG/1", sub3);
    engine.subscribe("Status/CRB/1", sub3);
    
    // Kreiranje publisher-a
    Publisher* pub1 = new Publisher(1, &engine);
    Publisher* pub2 = new Publisher(2, &engine);
    
    // Pokretanje subscriber-a
    sub1->start();
    sub2->start();
    sub3->start();
    
    bool running = true;
    bool autoDemo = false;
    
    while (running) {
        printMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();  // Brisanje newline karaktera
        
        switch (choice) {
            case 1: {
                // Pokretanje automatskog demo-a
                if (!autoDemo) {
                    std::cout << "\nPokretanje automatskog demo-a..." << std::endl;
                    std::cout << "Publisher-i ce automatski slati poruke na svakih 2 sekunde." << std::endl;
                    std::cout << "Pritisnite Enter da zaustavite demo..." << std::endl;
                    
                    pub1->start();
                    pub2->start();
                    autoDemo = true;
                    
                    std::cin.get();  // Cekanje na Enter
                    
                    pub1->stop();
                    pub2->stop();
                    autoDemo = false;
                    
                    std::cout << "Demo zaustavljen." << std::endl;
                } else {
                    std::cout << "Demo je vec pokrenut!" << std::endl;
                }
                break;
            }
            
            case 2: {
                // Rucno slanje poruke
                printTopicMenu();
                int msgChoice;
                std::cin >> msgChoice;
                std::cin.ignore();
                
                Message msg;
                msg.timestamp = std::time(nullptr);
                
                if (msgChoice == 1) {
                    // Analog poruka
                    strncpy(msg.topic, "Analog/MER/220", 63);
                    msg.type = MessageType::ANALOG;
                    msg.topicType = TopicType::MER;
                    
                    std::cout << "Unesite analog vrednost (npr. 220.5): ";
                    std::cin >> msg.data.analogValue;
                    std::cin.ignore();
                }
                else if (msgChoice == 2) {
                    // Switchgear status
                    strncpy(msg.topic, "Status/SWG/1", 63);
                    msg.type = MessageType::STATUS;
                    msg.topicType = TopicType::SWG;
                    
                    std::cout << "Unesite status (0=OPEN, 1=CLOSED): ";
                    int status;
                    std::cin >> status;
                    std::cin.ignore();
                    
                    msg.data.statusValue = (status == 0) ? StatusValue::SWG_OPEN : StatusValue::SWG_CLOSED;
                }
                else if (msgChoice == 3) {
                    // Circuit breaker status
                    strncpy(msg.topic, "Status/CRB/1", 63);
                    msg.type = MessageType::STATUS;
                    msg.topicType = TopicType::CRB;
                    
                    std::cout << "Unesite status (0=OPEN, 1=CLOSED): ";
                    int status;
                    std::cin >> status;
                    std::cin.ignore();
                    
                    msg.data.statusValue = (status == 0) ? StatusValue::CRB_OPEN : StatusValue::CRB_CLOSED;
                }
                else {
                    std::cout << "Pogresan izbor!" << std::endl;
                    break;
                }
                
                // Objavljivanje poruke
                Publisher manualPub(99, &engine);
                manualPub.publish(msg);
                
                std::cout << "\nPoruka objavljena! Proverite iznad za potvrdu isporuke." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                break;
            }
            
            case 3: {
                // Prikaz svih topic-a
                char topicList[100][64];
                int count;
                engine.getAllTopics(topicList, count, 100);
                
                std::cout << "\n=== Aktivni Topic-i ===" << std::endl;
                if (count == 0) {
                    std::cout << "Nema aktivnih topic-a." << std::endl;
                } else {
                    for (int i = 0; i < count; i++) {
                        std::cout << (i+1) << ". " << topicList[i] << std::endl;
                    }
                }
                break;
            }
            
            case 4: {
                // Prikaz broja subscriber-a
                std::cout << "\n=== Broj Subscriber-a ===" << std::endl;
                std::cout << "Topic 'Analog/MER/220': " 
                         << engine.getSubscriberCount("Analog/MER/220") << " subscriber(a)" << std::endl;
                std::cout << "Topic 'Status/SWG/1': " 
                         << engine.getSubscriberCount("Status/SWG/1") << " subscriber(a)" << std::endl;
                std::cout << "Topic 'Status/CRB/1': " 
                         << engine.getSubscriberCount("Status/CRB/1") << " subscriber(a)" << std::endl;
                break;
            }
            
            case 5: {
                // Izlaz
                std::cout << "\nGasenje programa..." << std::endl;
                running = false;
                break;
            }
            
            default: {
                std::cout << "Pogresan izbor! Pokusajte ponovo." << std::endl;
                break;
            }
        }
        
        if (running) {
            std::cout << "\nPritisnite Enter za nastavak...";
            std::cin.get();
        }
    }
    
    // Zaustavljanje publisher-a ako rade
    if (autoDemo) {
        pub1->stop();
        pub2->stop();
    }
    
    // Zaustavljanje subscriber-a
    sub1->stop();
    sub2->stop();
    sub3->stop();
    
    // Oslobadjanje memorije
    delete pub1;
    delete pub2;
    delete sub1;
    delete sub2;
    delete sub3;
    
    std::cout << "Program uspesno zavrsen." << std::endl;
    
    return 0;
}
*/

#include "PubSubEngine.h"
#include "Publisher.h"
#include "Subscriber.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <ctime>

struct SubscriberInfo {
    Subscriber* sub;
    std::vector<std::string> topics;
};

void printMenu() {
    std::cout << "\n========== PubSub System Console ==========\n";
    std::cout << "1. Dodaj Publishera\n";
    std::cout << "2. Dodaj Subscribera\n";
    std::cout << "3. Lista Publishera\n";
    std::cout << "4. Lista Subscribera + Topici\n";
    std::cout << "5. Lista Topica\n";
    std::cout << "6. Obrisi Publishera\n";
    std::cout << "7. Obrisi Subscribera\n";
    std::cout << "8. Odjavi Subscribera (po topicu)\n";
    std::cout << "9. Dodaj pretplatu Subscriberu\n";
    std::cout << "10. Posalji Poruku\n";
    std::cout << "11. Izlaz\n";
    std::cout << "===========================================\n";
    std::cout << "Izbor: ";
}

int main() {

    PubSubEngine engine;

    std::vector<Publisher*> publishers;
    std::vector<SubscriberInfo> subscribers;

    int nextPubID = 1;
    int nextSubID = 1;

    bool running = true;

    while(running) {

        printMenu();

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch(choice) {

        case 1: {
            Publisher* p = new Publisher(nextPubID++, &engine);
            publishers.push_back(p);
            std::cout<<"Publisher dodat.\n";
            break;
        }

        case 2: {
            Subscriber* s = new Subscriber(nextSubID++);
            s->start();

            SubscriberInfo info;
            info.sub = s;

            int t;
            while(true){
                std::cout<<"Izbor topica:\n";
                std::cout<<"1 Analog/MER/220\n";
                std::cout<<"2 Status/SWG/1\n";
                std::cout<<"3 Status/CRB/1\n";
                std::cout<<"4 Sva tri\n";
                std::cin>>t;

                if(t>=1 && t<=4) break;
                std::cout<<"Greska! Pogresan izbor.\n";
            }

            if(t==1||t==4){
                engine.subscribe("Analog/MER/220",s);
                info.topics.push_back("Analog/MER/220");
            }
            if(t==2||t==4){
                engine.subscribe("Status/SWG/1",s);
                info.topics.push_back("Status/SWG/1");
            }
            if(t==3||t==4){
                engine.subscribe("Status/CRB/1",s);
                info.topics.push_back("Status/CRB/1");
            }

            subscribers.push_back(info);
            break;
        }

        case 3:{
            for(auto p:publishers)
                std::cout<<"Publisher ID "<<p->getId()<<"\n";
            break;
        }

        case 4:{
            for(auto &s:subscribers){
                std::cout<<"Subscriber "<<s.sub->getId()<<": ";
                for(auto &t:s.topics) std::cout<<t<<" ";
                std::cout<<"\n";
            }
            break;
        }

        case 5:{
            char list[100][64];
            int count;
            engine.getAllTopics(list,count,100);
            for(int i=0;i<count;i++)
                std::cout<<list[i]<<"\n";
            break;
        }

        case 6:{
            int id; std::cout<<"ID:"; std::cin>>id;
            publishers.erase(
                std::remove_if(publishers.begin(),publishers.end(),
                    [&](Publisher* p){
                        if(p->getId()==id){delete p;return true;}
                        return false;
                    }),
                publishers.end());
            break;
        }

        case 7:{
            int id; std::cout<<"ID:"; std::cin>>id;
            subscribers.erase(
                std::remove_if(subscribers.begin(),subscribers.end(),
                    [&](SubscriberInfo &s){
                        if(s.sub->getId()==id){
                            s.sub->stop();
                            delete s.sub;
                            return true;
                        }
                        return false;
                    }),
                subscribers.end());
            break;
        }

        case 8:{ // ODJAVA SA POJEDINACNOG TOPICA
            int id; std::cout<<"ID:"; std::cin>>id;

            for(auto &s:subscribers){
                if(s.sub->getId()==id){

                    if(s.topics.empty()){
                        std::cout<<"Nema aktivnih pretplata\n";
                        break;
                    }

                    for(size_t i=0;i<s.topics.size();i++)
                        std::cout<<i+1<<" "<<s.topics[i]<<"\n";

                    int izbor;
                    std::cin>>izbor;

                    if(izbor>=1 && izbor<=s.topics.size()){
                        engine.unsubscribe(
                            s.topics[izbor-1].c_str(),
                            s.sub
                        );
                        s.topics.erase(s.topics.begin()+izbor-1);
                    }
                    else{
                        std::cout<<"Greska!\n";
                    }
                }
            }
            break;
        }

        case 9:{ // DODAJ PRETPLATU
            int id; std::cout<<"ID:"; std::cin>>id;

            for(auto &s:subscribers){
                if(s.sub->getId()==id){

                    std::cout<<"1 Analog/MER/220\n";
                    std::cout<<"2 Status/SWG/1\n";
                    std::cout<<"3 Status/CRB/1\n";

                    int t; std::cin>>t;

                    const char* topic=nullptr;

                    if(t==1) topic="Analog/MER/220";
                    else if(t==2) topic="Status/SWG/1";
                    else if(t==3) topic="Status/CRB/1";
                    else{
                        std::cout<<"Greska!\n";
                        break;
                    }

                    engine.subscribe(topic,s.sub);
                    s.topics.push_back(topic);

                    std::cout<<"Pretplata dodata.\n";
                }
            }
            break;
        }

        case 10:{
            if(publishers.empty()){
                std::cout<<"Nema publishera!\n";
                break;
            }

            std::cout<<"ID Publishera: ";
            int id; std::cin>>id;

            Publisher* sender=nullptr;
            for(auto p:publishers)
                if(p->getId()==id) sender=p;

            if(!sender){
                std::cout<<"Ne postoji publisher\n";
                break;
            }

            Message msg;
            msg.timestamp=std::time(nullptr);

            int t;
            while(true){
                std::cout<<"1 Analog\n2 SWG\n3 CRB\nIzbor:";
                std::cin>>t;
                if(t>=1&&t<=3) break;
                std::cout<<"Greska!\n";
            }

            if(t==1){
                strcpy(msg.topic,"Analog/MER/220");
                msg.type=MessageType::ANALOG;
                msg.topicType=TopicType::MER;
                std::cout<<"Vrednost:"; std::cin>>msg.data.analogValue;
            }
            else{
                int v;
                while(true){
                    std::cout<<"1 OPEN\n2 CLOSED\n";
                    std::cin>>v;
                    if(v==1||v==2) break;
                    std::cout<<"Greska!\n";
                }

                msg.type=MessageType::STATUS;

                if(t==2){
                    strcpy(msg.topic,"Status/SWG/1");
                    msg.topicType=TopicType::SWG;
                    msg.data.statusValue=(v==1)?
                        StatusValue::SWG_OPEN:
                        StatusValue::SWG_CLOSED;
                }else{
                    strcpy(msg.topic,"Status/CRB/1");
                    msg.topicType=TopicType::CRB;
                    msg.data.statusValue=(v==1)?
                        StatusValue::CRB_OPEN:
                        StatusValue::CRB_CLOSED;
                }
            }

            sender->publish(msg);

            std::cout<<"Publisher "<<sender->getId()
                     <<" poslao poruku na "<<msg.topic<<"\n";
            break;
        }

        case 11:
            running=false;
            break;

        default:
            std::cout<<"Nepoznata komanda\n";
        }
    }

    for(auto p:publishers) delete p;
    for(auto &s:subscribers){
        s.sub->stop();
        delete s.sub;
    }

    std::cout<<"Program zavrsen.\n";
    return 0;
}
