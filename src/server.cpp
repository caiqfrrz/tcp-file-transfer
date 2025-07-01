#include "server.h"

enum req_type
{
    file,
    message,
    quit,
    help,
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
        if (req == "h" || req == "help")
            return help;
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

        ClientInfo ci = acceptClientConnection();
        if (ci.socket < 0)
        {
            continue;
        }

        std::thread([this, clientSocket = ci.socket, clientAddr = ci.addr]
                    {
                        this->handleInput(clientSocket, clientAddr);

                        {
                            std::lock_guard<std::mutex> lock(lockVector);
                            auto it = std::find_if(clients.begin(), clients.end(),
                                [clientSocket](const ClientInfo& ci) {
                                    return ci.socket == clientSocket;
                                });

                            if (it != clients.end())
                            {
                                clients.erase(it);
                            }
                        } })
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

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEADDR) failed.\n";
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

ClientInfo Server::acceptClientConnection()
{
    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientSocket < 0)
    {
        std::cerr << "Accept failed: " << std::endl;
        return ClientInfo{-1, sockaddr_in{}};
    }
    std::cout << "Accepted a connection from client" << std::endl;

    {
        std::lock_guard<std::mutex> lock(lockVector);
        clients.push_back(ClientInfo{clientSocket, clientAddr});
    }

    return ClientInfo{clientSocket, clientAddr};
}

void Server::sendMenu(int clientSocket)
{
    const std::string menu =
        R"(
file <file-name>          -> download a file
message <message-data>    -> send a message
list                      -> list the files in the server
help                      -> this screen
q or quit                 -> quit
)";
    send(clientSocket, menu.c_str(), menu.size(), 0);
}

void Server::handleInput(int clientSocket, sockaddr_in clientAddr)
{
    try
    {
        this->sendMenu(clientSocket);
        while (true)
        {
            std::string req = receiveData(clientSocket);
            if (req.empty())
            {
                std::cerr << "Connection closed by client.\n";
                break;
            }

            req_type type = getRequestType(req);

            switch (type)
            {
            case file:
            {
                std::string filename = req.substr(5);
                sendFile(filename, clientSocket);
                break;
            }
            case message:
            {
                std::string messageStr = req.substr(8);
                std::string clientIp = inet_ntoa(clientAddr.sin_addr);
                uint16_t clientPort = ntohs(clientAddr.sin_port);

                std::string messageComplete = "MSG " + clientIp + " says: " + messageStr + "\n";

                std::cout << "broadcasting " + messageComplete << std::endl;

                broadcastMessage(messageComplete, clientSocket);
                break;
            }
            case quit:
                break;
            case help:
                sendMenu(clientSocket);
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
        std::string err = "MSG ERR: file not found.\n";
        send(clientSocket, err.c_str(), err.size(), 0);
        return false;
    }

    // CORREÇÃO AQUI
    file.seekg(0, std::ios::end);
    std::streamsize filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string header = "FILE_START " + fileName + " " + std::to_string(filesize) + "\n";
    send(clientSocket, header.c_str(), header.size(), 0);

    std::cout << "Sending " << fileName << " (" << filesize << " bytes) to client." << std::endl;

    char buffer[4096];
    while (filesize > 0)
    {
        file.read(buffer, sizeof(buffer));
        std::streamsize bytesRead = file.gcount();

        int bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent != bytesRead)
        {
            std::cerr << "Send failed." << std::endl;
            file.close();
            return false;
        }

        filesize -= bytesRead;
    }

    file.close();
    std::string endMarker = "FILE_END\n";
    send(clientSocket, endMarker.c_str(), endMarker.size(), 0);

    std::cout << "File " << fileName << " successfully sent to client." << std::endl;
    return true;
}

void Server::broadcastMessage(const std::string &message, int senderSocket)
{
    std::lock_guard<std::mutex> lock(lockVector);
    for (ClientInfo client : clients)
    {
        send(client.socket, message.c_str(), message.size(), 0);
    }
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