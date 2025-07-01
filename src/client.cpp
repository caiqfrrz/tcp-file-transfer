#include "client.h"

Client::Client()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (!createSocket() || !connectServer())
        return;

    handleInteraction();
}

Client::~Client()
{
    if (clientSocket != -1)
        close(clientSocket);
}

bool Client::createSocket()
{
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        std::cerr << "Socket creation failed: " << std::endl;
        return false;
    }
    return true;
}

bool Client::connectServer()
{
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cerr << "Fail connecting to server" << std::endl;
        return false;
    }
    std::cout << "Connected to server" << std::endl;
    return true;
}

void Client::displayServerMenu()
{
    char buffer[1024];
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }
}

void Client::listenServer()
{
    while (true)
    {
        std::string line = receiveLine();
        if (line.empty())
        {
            std::cerr << "Disconnected from server." << std::endl;
            break;
        }

        if (line.rfind("MSG", 0) == 0)
        {
            std::cout << "\r";
            std::cout << "                                         \r";
            std::cout << line.substr(4) << std::endl;
            std::cout << "Enter command (h for help): " << std::flush;
        }
        else if (line.rfind("FILE_START", 0) == 0)
        {
            handleFileDownload(line);
        }
    }

    running = false;
    close(clientSocket);
}

std::string Client::receiveLine()
{
    std::string line;
    char ch;
    while (recv(clientSocket, &ch, 1, 0) > 0)
    {
        if (ch == '\n')
            break;
        line += ch;
    }
    return line;
}

void Client::handleInteraction()
{
    displayServerMenu();
    std::thread([this]()
                { this->listenServer(); })
        .detach();

    while (true)
    {
        std::string input;
        std::cout << "Enter command (h for help): ";
        std::getline(std::cin, input);

        if (send(clientSocket, input.c_str(), input.size(), 0) <= 0)
        {
            std::cerr << "Failed to send command" << std::endl;
            break;
        }

        if (input.rfind("quit", 0) == 0)
        {
            break;
        }
    }

    running = false;
    close(clientSocket);
}

void Client::handleFileDownload(std::string fileInfo)
{
    // protocol: FILE_START filename size
    std::istringstream iss(fileInfo);
    std::string tag, filename;
    int filesize;
    iss >> tag >> filename >> filesize;

    std::ofstream file("downloaded_" + filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Could not create file." << std::endl;
        return;
    }

    int remaining = filesize;
    char buffer[4096];
    while (remaining > 0)
    {
        int chunk = recv(clientSocket, buffer, std::min((int)sizeof(buffer), remaining), 0);
        if (chunk <= 0)
        {
            std::cerr << "Connection lost during file transfer." << std::endl;
            break;
        }
        file.write(buffer, chunk);
        remaining -= chunk;
    }

    file.close();
    // read FILE_END
    receiveLine();

    std::cout << "File " << filename << " downloaded!" << std::endl;
}

int main()
{
    Client c;
    return 0;
}