#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <winsock2.h>
#include <string>
#include <thread>
#include <atomic>

// Forward declaration - izbjegava cirkularne include-ove
class PubSubEngine;

class HttpServer {
private:
    SOCKET      listenSocket;
    int         port;
    std::atomic<bool> running;
    std::thread serverThread;
    PubSubEngine* engine;

    void serveLoop();
    void handleClient(SOCKET client);
    static std::string getHtmlPage();

public:
    // eng  - pokazivac na engine za dohvatanje statistika
    // httpPort - port na kom ce se sluziti dashboard (default 8080)
    HttpServer(PubSubEngine* eng, int httpPort = 8080);
    ~HttpServer();

    void start();
    void stop();
};

#endif // HTTP_SERVER_H