#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fstream>

class Client
{
private:
    int clientSocket;
    sockaddr_in serverAddress;
    const int serverPort = 8000;

public:
    Client();
    ~Client();
    void displayServerMenu();
    void handleFileDownload(std::string fileName);
    bool createSocket();
    bool connectServer();
    void handleInteraction();
};