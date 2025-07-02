#include "server.h"

enum req_type
{
    file,
    message,
    quit,
    help,
    list,
    null
};

req_type getRequestType(const std::string &input)
{
    std::istringstream iss(input);
    std::string req;
    iss >> req;

    if (req == "file")
        return file;
    if (req == "message")
        return message;
    if (req == "list")
        return list;
    if (req == "q" || req == "quit")
        return quit;
    if (req == "h" || req == "help")
        return help;

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

    std::thread([this]()
                {
        while (running)
        {
            std::cout << "Send a message (quit to stop server): ";
            std::string input;
            std::getline(std::cin, input);
            if (input == "quit" || input == "exit")
            {
                std::cout << "Stopping server..." << std::endl;
                stop();
                break;
            }
            else if (!input.empty())
            {
                std::string message = "MSG SERVER says: " + input + "\n";
                broadcastMessage(message, -1);
                std::cout << message.substr(4);
            }
        } })
        .detach();

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
    printTerminal("Accepted a connection from client " + std::string(inet_ntoa(clientAddr.sin_addr)));

    {
        std::lock_guard<std::mutex> lock(lockVector);
        clients.push_back(ClientInfo{clientSocket, clientAddr});
    }

    return ClientInfo{clientSocket, clientAddr};
}

void Server::printTerminal(std::string str, bool err)
{
    std::cout << "\r" << std::string(80, ' ') << "\r";
    if (!err)
        std::cout << str << std::endl;
    else
        std::cerr << str << std::endl;
}

void Server::sendMenu(int clientSocket)
{
    const std::string menu =
        "MSG \n"
        "MSG file <file-name>          -> download a file\n"
        "MSG message <message-data>    -> send a message to all users\n"
        "MSG list                      -> list the files in the server\n"
        "MSG help                      -> this screen\n"
        "MSG q or quit                 -> quit\n"
        "MSG \n";

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
                printTerminal("Connection closed by client " + std::string(inet_ntoa(clientAddr.sin_addr)));
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

                printTerminal(messageComplete.substr(4));

                broadcastMessage(messageComplete, clientSocket);
                break;
            }
            case list:
            {
                for (const auto &entry : std::filesystem::directory_iterator("."))
                {
                    if (std::filesystem::is_regular_file(entry))
                    {
                        std::string line = "MSG - " + entry.path().filename().string() + "\n";
                        send(clientSocket, line.c_str(), line.size(), 0);
                    }
                }
                break;
            }
            break;
            case quit:
                return;
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

    file.seekg(0, std::ios::end);
    std::streamsize filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string fileHash = sha256(fileName);

    // example: FILE_START teste.txt 13 05da8b41f3f8537fa09b202680136f9bc0a06f8841b661ae0a9ca26ef72e62a3\n
    std::string header = "FILE_START " + fileName + " " + std::to_string(filesize) + " " + fileHash + "\n";
    send(clientSocket, header.c_str(), header.size(), 0);

    printTerminal("Sending " + fileName + " (" + std::to_string(filesize) + " bytes) to client.");

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

    printTerminal("File " + fileName + " successfully sent to client.");
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
        printTerminal("Receive failed", true);
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