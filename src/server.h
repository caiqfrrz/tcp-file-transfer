#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>

class Server
{
private:
    std::atomic<bool> running;
    sockaddr_in serverAddress;
    int serverSocket;
    const uint16_t PORT = 8000;

public:
    Server();
    ~Server();
    void stop();
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    std::string receiveData(int clientSocket);
    void handleInput(int clientSocket);
    void sendMenu(int clientSocket);
    bool sendFile(std::string fileName, int clientSocket);
    int acceptClientConnection();
};