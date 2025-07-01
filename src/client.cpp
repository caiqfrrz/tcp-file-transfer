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
        std::cout << "\nServer Menu:\n"
                  << buffer << std::endl;
    }
}

void Client::handleInteraction()
{
    displayServerMenu();

    while (true)
    {
        std::string input;
        std::cout << "Enter command (file <filename>, message <text>, quit): ";
        std::getline(std::cin, input);

        // Send the raw command to server
        if (send(clientSocket, input.c_str(), input.size(), 0) <= 0)
        {
            std::cerr << "Failed to send command" << std::endl;
            break;
        }

        // Handle different commands
        if (input.rfind("quit", 0) == 0)
        {
            break;
        }
        else if (input.rfind("file ", 0) == 0)
        {
            handleFileDownload(input.substr(5)); // Extract filename
        }
        else if (input.rfind("message ", 0) == 0)
        {
            // Message handling can be added later
            std::cout << "Message sent to server" << std::endl;
        }
    }
}

void Client::handleFileDownload(std::string fileName)
{
    std::ofstream file("downloaded" + fileName, std::ios::binary);
    if (!file)
    {
        std::cout << "Failed to open file." << std::endl;
        return;
    }

    char buffer[4096];
    u_int32_t bytesReceived = 0;
    std::cout << "Writing file to received_" << fileName << std::endl;

    while (bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0))
    {
        file.write(buffer, bytesReceived);

        if (bytesReceived < sizeof(buffer))
            break;
    }

    file.close();
    std::cout << "File received" << std::endl;
}

int main()
{
    Client c;
    return 0;
}