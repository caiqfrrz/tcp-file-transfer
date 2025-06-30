#include "server.h"

Server::Server()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (!createSocket() || !bindSocket() || !listenSocket())
    {
        return;
    }
    while (true)
    {
        if (!acceptClientConnection())
        {
            continue;
        }

        if (!receiveData())
        {
            std::cerr << "Error handling client request" << std::endl;
        }

        close(clientSocket);
        clientSocket = -1;
    }
}

Server::~Server()
{
    if (clientSocket != -1)
        close(clientSocket);
    if (serverSocket != -1)
        close(serverSocket);
}

bool Server::createSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        std::cerr << "Socket creation failed: " << std::endl;
        return false;
    }
    return true;
}

bool Server::bindSocket()
{
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cerr << "Bind failed: " << std::endl;
        return false;
    }
    return true;
}

bool Server::listenSocket()
{
    if (listen(serverSocket, 5) < 0)
    {
        std::cerr << "Listen failed: " << std::endl;
        return false;
    }
    std::cout << "Server listening on port " << PORT << std::endl;
    return true;
}

bool Server::acceptClientConnection()
{
    clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed: " << std::endl;
        return false;
    }
    std::cout << "Accepted a connection from client" << std::endl;
    return true;
}

bool Server::sendFile(std::string fileName)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        std::cout << "File " << fileName << " does not exist." << std::endl;
        return false;
    }

    std::cout << "Sending " << fileName << " to client:" << std::endl;

    char buffer[4096];
    u_int32_t i = 0;
    while (file.read(buffer, sizeof(buffer)))
    {
        int bytesRead = file.gcount();
        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent != bytesRead)
        {
            std::cerr << "Send failed" << std::endl;
            file.close();
            return false;
        }
        std::cout << "Sending chunk " << i++ << " to client" << std::endl;
    }

    if (file.gcount() > 0)
    {
        send(clientSocket, buffer, file.gcount(), 0);
    }

    std::cout << "File " << fileName << " was successfully sent to client." << std::endl;
    file.close();
    return true;
}

bool Server::receiveData()
{
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
    {
        std::cerr << "Receive failed" << std::endl;
        return false;
    }

    buffer[bytesReceived] = '\0';
    std::cout << "Requested: " << buffer << std::endl;
    sendFile(buffer);
    return true;
}

int main()
{
    Server s;
    return 0;
}