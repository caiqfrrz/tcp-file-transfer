#include "client.h"

Client::Client(const char *serverIp)
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) <= 0)
    {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return;
    }

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

void Client::listenServer()
{
    while (true)
    {
        std::string line = receiveLine();
        if (line.empty())
        {
            printTerminal("Disconnected from server.", true);
            break;
        }

        if (line.rfind("MSG", 0) == 0)
        {
            printTerminal(line.substr(4));
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
    std::thread([this]()
                { this->listenServer(); })
        .detach();

    while (true)
    {
        std::string input;
        std::cout << "Enter command: ";
        std::getline(std::cin, input);

        if (send(clientSocket, input.c_str(), input.size(), 0) <= 0)
        {
            printTerminal("Failed to send command", true);
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
    // protocol: FILE_START filename size hash
    std::istringstream iss(fileInfo);
    std::string tag, filename, fileHash;
    int filesize;
    iss >> tag >> filename >> filesize >> fileHash;

    std::ofstream file("downloaded_" + filename, std::ios::binary);
    if (!file)
    {
        printTerminal("Could not create file.", true);
        return;
    }

    int remaining = filesize;
    char buffer[4096];
    int downloaded = 0;
    while (remaining > 0)
    {
        int chunk = recv(clientSocket, buffer, std::min((int)sizeof(buffer), remaining), 0);
        if (chunk <= 0)
        {

            printTerminal("Connection lost during file transfer.", true);
            break;
        }
        file.write(buffer, chunk);
        downloaded += chunk;
        remaining -= chunk;

        int width = 50;
        float percent = (float)downloaded / filesize;
        int pos = width * percent;

        std::cout << "\r[";
        for (int i = 0; i < width; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(percent * 100) << "%";
        std::cout.flush();
    }

    file.close();

    // read FILE_END
    receiveLine();

    std::string localHash = sha256("downloaded_" + filename);
    printTerminal("File " + filename + " downloaded!");
    if (localHash == fileHash)
    {
        printTerminal("Integrity check OK: SHA256 matches.");
    }
    else
    {
        printTerminal("WARNING: SHA256 mismatch! File may be corrupted.", true);
    }
}

void Client::printTerminal(std::string str, bool err)
{
    std::cout << "\r" << std::string(80, ' ') << "\r";
    if (!err)
        std::cout << str << std::endl;
    else
        std::cerr << str << std::endl;

    std::cout << "Enter command: " << std::flush;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
        return 1;
    }

    Client c(argv[1]);
    return 0;
}