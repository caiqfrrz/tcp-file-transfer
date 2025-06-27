#include "client.h"

Client::Client()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = serverPort;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    createSocket();
    connectServer();
    sendData();

    close(clientSocket);
}

void Client::createSocket()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
}

void Client::connectServer()
{
    connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    std::cout << "Connected to server" << std::endl;
}

void Client::sendData()
{
    char message[1024];

    std::cout << "Enter your message: ";
    std::cin.getline(message, sizeof(message));
    std::cout << "Message sent: " << message << std::endl;

    send(clientSocket, message, strlen(message), 0);
}

int main()
{
    Client c;
    return 0;
}