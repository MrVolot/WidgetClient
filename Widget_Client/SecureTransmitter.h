#pragma once

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <optional>

class SecureTransmitter
{
    EVP_PKEY* privateKey_;
public:
    SecureTransmitter();
    std::vector<unsigned char> generateSharedKey(const std::string& userPublicKey);
    void setPrivateKey(const std::string& private_key_file);
    std::string encryptMessage(const std::string& message, std::vector<unsigned char> sharedKey);
    std::string decryptMessage(const std::string& encrypted_message, std::vector<unsigned char> sharedKey);
    EVP_PKEY* transformStringKey(const std::string& public_key_str);
    //std::optional<std::vector<unsigned char>> tryGetSharedKeyById(unsigned long long id);
    void print_openssl_error();
};

