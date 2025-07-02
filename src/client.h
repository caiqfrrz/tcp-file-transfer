#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <thread>
#include <sstream>
#include <atomic>
#include <openssl/sha.h>
#include "sha256sum.h"

class Client
{
private:
    int clientSocket;
    sockaddr_in serverAddress;
    const int serverPort = 8000;
    std::atomic<bool> running{true};

public:
    Client(const char *serverIp);
    ~Client();
    void handleFileDownload(std::string fileInfo);
    std::string receiveLine();
    bool createSocket();
    bool connectServer();
    void handleInteraction();
    void listenServer();
    void printTerminal(std::string str, bool err = false);
};