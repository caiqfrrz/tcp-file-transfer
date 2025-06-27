#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

class Client
{
private:
    int clientSocket;
    int serverSocket;
    sockaddr_in serverAddress;
    const int serverPort = htons(8000);

public:
    Client();
    void createSocket();
    void connectServer();
    void sendData();
};