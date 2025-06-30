#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

class Server
{
private:
    sockaddr_in serverAddress;
    int serverSocket;
    int clientSocket;
    const uint16_t PORT = 8000;

public:
    Server();
    ~Server();
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    bool receiveData();
    bool sendFile(std::string fileName);
    bool acceptClientConnection();
};