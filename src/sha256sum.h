#include <openssl/sha.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string sha256(const std::string &filename)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open file for SHA256 calculation.");
    }

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)))
    {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    if (file.gcount() > 0)
    {
        SHA256_Update(&sha256, buffer, file.gcount());
    }

    SHA256_Final(hash, &sha256);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}