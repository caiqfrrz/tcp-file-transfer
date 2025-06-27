#include "server.h"

Server::Server()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = PORT;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    createSocket();
    bindSocket();
    listenSocket();
    acceptClientConnection();
    receiveData();

    close(serverSocket);
}

void Server::createSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}

void Server::bindSocket()
{
    bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
}

void Server::listenSocket()
{
    listen(serverSocket, 5);
    std::cout << "Server listening on port " << PORT << std::endl;
}

void Server::acceptClientConnection()
{
    clientSocket = accept(serverSocket, nullptr, nullptr);
    std::cout << "Accepted a connection from client" << std::endl;
}

void Server::receiveData()
{
    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "Message from client: " << buffer << std::endl;
}

int main()
{
    Server s;
    return 0;
}