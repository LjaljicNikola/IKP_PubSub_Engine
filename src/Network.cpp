#include "Network.h"

// ==================== Console Handler ====================
std::atomic<bool> ConsoleHandler::exitRequested(false);
std::thread ConsoleHandler::inputThread;
bool ConsoleHandler::initialized = false;

// ==================== Port Pool ====================
int PortPool::publisherPortCounter = 0;
int PortPool::subscriberPortCounter = 0;
std::mutex PortPool::poolMutex;

// ==================== TCP Client ====================
bool TcpClient::wsInitialized = false;
std::mutex TcpClient::wsMutex;

// ==================== TCP Server ====================
bool TcpServer::wsInitialized = false;
std::mutex TcpServer::wsMutex;
