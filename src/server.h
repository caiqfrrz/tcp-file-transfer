#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

class Server
{
private:
    sockaddr_in serverAddress;
    int serverSocket;
    int clientSocket;
    const uint16_t PORT = htons(8000);

public:
    Server();
    void createSocket();
    void bindSocket();
    void listenSocket();
    void receiveData();
    void acceptClientConnection();
};