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
    const int serverPort = htons(8000);

public:
    Client();
    ~Client();
    bool createSocket();
    bool connectServer();
    void requestAndSaveFile();
};