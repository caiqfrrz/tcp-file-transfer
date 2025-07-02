#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <bits/stdc++.h>
#include <filesystem>
#include "sha256sum.h"

struct ClientInfo
{
    int socket;
    sockaddr_in addr;

    inline bool operator==(const ClientInfo &c) const
    {
        return socket == c.socket && addr.sin_addr.s_addr == c.addr.sin_addr.s_addr;
    }
};

class Server
{
private:
    std::atomic<bool> running;
    sockaddr_in serverAddress;
    int serverSocket;
    const uint16_t PORT = 8000;

    std::vector<ClientInfo> clients;
    std::mutex lockVector;

public:
    Server();
    ~Server();
    void stop();
    bool createSocket();
    bool bindSocket();
    bool listenSocket();
    std::string receiveData(int clientSocket);
    void handleInput(int clientSocket, sockaddr_in clientAdd);
    void sendMenu(int clientSocket);
    bool sendFile(std::string fileName, int clientSocket);
    void broadcastMessage(const std::string &message, int senderSocket);
    void printTerminal(std::string str, bool err = false);
    ClientInfo acceptClientConnection();
};
