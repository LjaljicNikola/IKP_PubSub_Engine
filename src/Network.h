#ifndef NETWORK_H
#define NETWORK_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <cstring>
#include <iostream>
#include <chrono>
#include <algorithm>

// ==================== Console Handler ====================
class ConsoleHandler {
private:
    static std::atomic<bool> exitRequested;
    static std::thread inputThread;
    static bool initialized;
    
    static void readInput() {
        std::string line;
        while (true) {
            try {
                if (std::getline(std::cin, line)) {
                    if (line == "exit" || line == "EXIT") {
                        std::cout << "\nShutdown requested..." << std::endl;
                        exitRequested.store(true);
                        break;
                    }
                } else {
                    // stdin closed or error (e.g., no console)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& e) {
                // Handle any input exceptions gracefully
                std::cerr << "Input thread error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
public:
    static void init() {
        if (!initialized) {
            initialized = true;
            exitRequested.store(false);
            inputThread = std::thread(readInput);
            inputThread.detach();
        }
    }
    
    static bool shouldExit() {
        return exitRequested.load();
    }
};


// ==================== Port Pool ====================
class PortPool {
private:
    static int publisherPortCounter;
    static int subscriberPortCounter;
    static std::mutex poolMutex;
    
    static const int PUBLISHER_BASE = 4100;
    static const int SUBSCRIBER_BASE = 4200;
    
public:
    static int getNextPublisherPort() {
        std::lock_guard<std::mutex> lock(poolMutex);
        return PUBLISHER_BASE + (publisherPortCounter++);
    }
    
    static int getNextSubscriberPort() {
        std::lock_guard<std::mutex> lock(poolMutex);
        return SUBSCRIBER_BASE + (subscriberPortCounter++);
    }
    
    static int getEnginePort() {
        return 5000;
    }
};


// ==================== TCP Client ====================
class TcpClient {
private:
    SOCKET socket;
    std::string hostAddr;
    int port;
    bool connected;
    
    static bool wsInitialized;
    static std::mutex wsMutex;
    
    static void initWinsock() {
        std::lock_guard<std::mutex> lock(wsMutex);
        if (!wsInitialized) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                std::cerr << "WSAStartup failed" << std::endl;
            }
            wsInitialized = true;
        }
    }
    
public:
    TcpClient() : socket(INVALID_SOCKET), port(0), connected(false) {
        initWinsock();
    }
    
    ~TcpClient() {
        disconnect();
    }
    
    bool connect(const std::string& host, int port_num) {
        hostAddr = host;
        port = port_num;
        
        struct addrinfo hints, *result = nullptr;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        
        std::string port_str = std::to_string(port);
        
        if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result) != 0) {
            std::cerr << "TcpClient: getaddrinfo failed for " << host << ":" << port << std::endl;
            return false;
        }
        
        socket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (socket == INVALID_SOCKET) {
            std::cerr << "TcpClient: socket creation failed" << std::endl;
            freeaddrinfo(result);
            return false;
        }
        
        // Set connection timeout
        DWORD timeout = 5000; // 5 seconds
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
        
        int connectResult = ::connect(socket, result->ai_addr, (int)result->ai_addrlen);
        freeaddrinfo(result);
        
        if (connectResult == SOCKET_ERROR) {
            std::cerr << "TcpClient: connect failed to " << host << ":" << port << std::endl;
            closesocket(socket);
            socket = INVALID_SOCKET;
            return false;
        }
        
        connected = true;
        return true;
    }
    
    bool sendMessage(const std::vector<uint8_t>& data) {
        if (!connected || socket == INVALID_SOCKET) {
            return false;
        }
        
        // Send length prefix (4 bytes, big-endian)
        uint32_t len = data.size();
        uint8_t len_bytes[4] = {
            (uint8_t)((len >> 24) & 0xFF),
            (uint8_t)((len >> 16) & 0xFF),
            (uint8_t)((len >> 8) & 0xFF),
            (uint8_t)(len & 0xFF)
        };
        
        int sent = ::send(socket, (const char*)len_bytes, 4, 0);
        if (sent == SOCKET_ERROR) {
            return false;
        }
        
        // Send payload
        sent = ::send(socket, (const char*)data.data(), data.size(), 0);
        if (sent == SOCKET_ERROR) {
            return false;
        }
        
        return true;
    }
    
    std::vector<uint8_t> receiveMessage() {
        std::vector<uint8_t> result;
        
        if (!connected || socket == INVALID_SOCKET) {
            return result;
        }
        
        // Read length prefix
        uint8_t len_bytes[4];
        int received = ::recv(socket, (char*)len_bytes, 4, MSG_WAITALL);
        if (received != 4) {
            return result;
        }
        
        uint32_t len = ((uint32_t)len_bytes[0] << 24) |
                       ((uint32_t)len_bytes[1] << 16) |
                       ((uint32_t)len_bytes[2] << 8) |
                       (uint32_t)len_bytes[3];
        
        if (len > 10000) {
            std::cerr << "TcpClient: invalid message length" << std::endl;
            return result;
        }
        
        // Read payload
        result.resize(len);
        received = ::recv(socket, (char*)result.data(), len, MSG_WAITALL);
        if (received != (int)len) {
            result.clear();
            return result;
        }
        
        return result;
    }
    
    void disconnect() {
        if (socket != INVALID_SOCKET) {
            closesocket(socket);
            socket = INVALID_SOCKET;
        }
        connected = false;
    }
    
    bool isConnected() const {
        return connected && socket != INVALID_SOCKET;
    }
};


// ==================== TCP Server ====================
class TcpServer {
private:
    SOCKET listenSocket;
    int port;
    bool listening;
    std::thread acceptThread;
    std::atomic<bool> running;
    std::mutex clientSocketMutex;
    std::vector<SOCKET> clientSockets;  // Store multiple clients
    std::mutex messageQueueMutex;
    std::vector<std::vector<uint8_t>> messageQueue;
    
    static bool wsInitialized;
    static std::mutex wsMutex;
    
    static void initWinsock() {
        std::lock_guard<std::mutex> lock(wsMutex);
        if (!wsInitialized) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                std::cerr << "WSAStartup failed" << std::endl;
            }
            wsInitialized = true;
        }
    }
    
    void acceptLoop() {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        
        while (running.load()) {
            SOCKET client = ::accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (client != INVALID_SOCKET) {
                {
                    std::lock_guard<std::mutex> lock(clientSocketMutex);
                    clientSockets.push_back(client);
                }
                
                // Spawn thread to handle this client
                std::thread(&TcpServer::handleClient, this, client).detach();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    
    void handleClient(SOCKET client) {
        while (running.load()) {
            // Read length prefix
            uint8_t len_bytes[4];
            int received = ::recv(client, (char*)len_bytes, 4, MSG_WAITALL);
            if (received != 4) {
                closesocket(client);
                
                {
                    std::lock_guard<std::mutex> lock(clientSocketMutex);
                    auto it = std::find(clientSockets.begin(), clientSockets.end(), client);
                    if (it != clientSockets.end()) {
                        clientSockets.erase(it);
                    }
                }
                return;
            }
            
            uint32_t len = ((uint32_t)len_bytes[0] << 24) |
                           ((uint32_t)len_bytes[1] << 16) |
                           ((uint32_t)len_bytes[2] << 8) |
                           (uint32_t)len_bytes[3];
            
            if (len > 10000) {
                closesocket(client);
                
                {
                    std::lock_guard<std::mutex> lock(clientSocketMutex);
                    auto it = std::find(clientSockets.begin(), clientSockets.end(), client);
                    if (it != clientSockets.end()) {
                        clientSockets.erase(it);
                    }
                }
                return;
            }
            
            // Read payload
            std::vector<uint8_t> payload(len);
            received = ::recv(client, (char*)payload.data(), len, MSG_WAITALL);
            if (received != (int)len) {
                closesocket(client);
                
                {
                    std::lock_guard<std::mutex> lock(clientSocketMutex);
                    auto it = std::find(clientSockets.begin(), clientSockets.end(), client);
                    if (it != clientSockets.end()) {
                        clientSockets.erase(it);
                    }
                }
                return;
            }
            
            {
                std::lock_guard<std::mutex> lock(messageQueueMutex);
                messageQueue.push_back(payload);
            }
        }
    }
    
public:
    TcpServer() : listenSocket(INVALID_SOCKET), port(0), listening(false), running(false) {
        initWinsock();
    }
    
    ~TcpServer() {
        stop();
    }
    
    bool start(int port_num) {
        port = port_num;
        
        listenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listenSocket == INVALID_SOCKET) {
            std::cerr << "TcpServer: socket creation failed" << std::endl;
            return false;
        }
        
        // Allow socket reuse
        int reuse = 1;
        if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
            std::cerr << "TcpServer: setsockopt SO_REUSEADDR failed" << std::endl;
        }
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
        serverAddr.sin_port = htons(port);
        
        if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "TcpServer: bind failed on port " << port << std::endl;
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
            return false;
        }
        
        if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "TcpServer: listen failed" << std::endl;
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
            return false;
        }
        
        listening = true;
        running.store(true);
        acceptThread = std::thread(&TcpServer::acceptLoop, this);
        acceptThread.detach();
        
        return true;
    }
    
    std::vector<uint8_t> receiveMessage() {
        std::vector<uint8_t> result;
        
        {
            std::lock_guard<std::mutex> lock(messageQueueMutex);
            if (!messageQueue.empty()) {
                result = messageQueue.front();
                messageQueue.erase(messageQueue.begin());
            }
        }
        
        if (result.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        return result;
    }
    
    bool sendMessage(const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lock(clientSocketMutex);
        
        if (clientSockets.empty()) {
            return false;
        }
        
        // Send to the first connected client (or could be improved to track specific clients)
        SOCKET client = clientSockets[0];
        
        // Send length prefix
        uint32_t len = data.size();
        uint8_t len_bytes[4] = {
            (uint8_t)((len >> 24) & 0xFF),
            (uint8_t)((len >> 16) & 0xFF),
            (uint8_t)((len >> 8) & 0xFF),
            (uint8_t)(len & 0xFF)
        };
        
        int sent = ::send(client, (const char*)len_bytes, 4, 0);
        if (sent == SOCKET_ERROR) {
            return false;
        }
        
        // Send payload
        sent = ::send(client, (const char*)data.data(), data.size(), 0);
        if (sent == SOCKET_ERROR) {
            return false;
        }
        
        return true;
    }
    
    void stop() {
        running.store(false);
        
        {
            std::lock_guard<std::mutex> lock(clientSocketMutex);
            for (SOCKET client : clientSockets) {
                if (client != INVALID_SOCKET) {
                    closesocket(client);
                }
            }
            clientSockets.clear();
        }
        
        if (listenSocket != INVALID_SOCKET) {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        
        listening = false;
    }
    
    int getPort() const {
        return port;
    }
};

#endif // NETWORK_H
