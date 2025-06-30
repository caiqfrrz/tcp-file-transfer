#include "client.h"

Client::Client()
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = serverPort;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (!createSocket() || !connectServer())
        return;

    requestAndSaveFile();
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

void Client::requestAndSaveFile()
{
    std::string fileName;

    while (true)
    {
        std::cout << "Enter file name (or 'q' to quit): ";
        std::getline(std::cin, fileName);

        for (char &c : fileName)
        {
            c = tolower(static_cast<unsigned char>(c));
        }

        if (fileName == "q")
        {
            break;
        }

        send(clientSocket, fileName.c_str(), fileName.size(), 0);

        std::ofstream file("received_" + fileName, std::ios::binary);
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
}

int main()
{
    Client c;
    return 0;
}