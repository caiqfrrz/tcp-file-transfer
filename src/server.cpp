#include "server.h"

enum req_type
{
    file,
    message,
    quit,
    null
};

req_type getRequestType(const std::string &input)
{
    size_t space_pos = input.find(' ');
    if (space_pos != std::string::npos)
    {
        std::string req = input.substr(0, space_pos);

        if (req == "file")
            return file;
        if (req == "message")
            return message;
        if (req == "q" || req == "quit")
            return quit;
    }
    return null;
}

Server::Server() : running(true)
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (!createSocket() || !bindSocket() || !listenSocket())
    {
        return;
    }
    while (running)
    {
        int cs = acceptClientConnection();
        if (cs < 0)
        {
            continue;
        }
        std::thread([this, clientSocket = cs] { // C++17 capture with initialization
            this->handleInput(clientSocket);
        })
            .detach();
    }
}

Server::~Server()
{
    if (serverSocket != -1)
        close(serverSocket);
}

void Server::stop()
{
    this->running = false;
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

int Server::acceptClientConnection()
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed: " << std::endl;
        return -1;
    }
    std::cout << "Accepted a connection from client" << std::endl;
    return clientSocket;
}

void Server::sendMenu(int clientSocket)
{
    std::string menu = "1. Download file\n2. Upload file\n3. List files\n4. Exit\n";
    send(clientSocket, menu.c_str(), menu.size(), 0);
}

void Server::handleInput(int clientSocket)
{
    try
    {
        this->sendMenu(clientSocket);
        while (true)
        {
            std::string req = receiveData(clientSocket);
            if (req.empty())
                break;

            req_type type = getRequestType(req);

            switch (type)
            {
            case file:
            {
                std::string filename = req.substr(5);
                // TODO: trim whitespace
                sendFile(filename, clientSocket);
                break;
            }
            case message:
                break;
            case quit:
                break;
            default:
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client handling error: " << e.what() << std::endl;
    }
    close(clientSocket);
}

bool Server::sendFile(std::string fileName, int clientSocket)
{
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
    {
        std::string err = "File " + fileName + " does not exist.\n";
        send(clientSocket, err.c_str(), err.size(), 0);
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

std::string Server::receiveData(int clientSocket)
{
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
    {
        std::cerr << "Receive failed" << std::endl;
        return "";
    }

    buffer[bytesReceived] = '\0';
    std::string str(buffer);
    return str;
}

int main()
{
    Server s;
    return 0;
}