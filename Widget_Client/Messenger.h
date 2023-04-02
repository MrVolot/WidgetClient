#pragma once

#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <Contact.h>
#include "../Json/json/json.h"
#include <Commands.h>
#include <ContactsWidget.h>
#include "Config.h"

// Include necessary OpenSSL headers
#pragma warning(disable : 4996)
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

template <typename Caller>
class Messenger: public std::enable_shared_from_this<Messenger<Caller>>
{
    std::string ip_;
    short port_;
    boost::asio::io_service& service_;
    std::shared_ptr<IConnectionHandler<Messenger>> handler_;
    unsigned long long userId_;
    std::string hash_;
    std::string name_;
    Caller* caller_;
    Config config_;
    std::vector<Contact> friendList_;
    std::vector<std::map<std::string, QString>> chatHistoryVector_;
    std::vector<Contact> possibleContactsVector_;

    std::function<void(Caller*, const QString&, unsigned long long)> senderCallback_;

    void readCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void writeCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    void parseServerCommands(const std::string& data);
    void receiveMessage(const std::string& data);
    void fillFriendList(const std::string& jsonData);
    void fillChatHistory(const std::string& jsonData);
    void parsePossibleContacts(const std::string& jsonData);


    void key_exchange();
    std::string encrypt_message(const std::string& message);
    std::string decrypt_message(const std::string& encrypted_message);
    EVP_PKEY* send_and_receive_public_keys(EVP_PKEY* client_public_key);

    // Add necessary member variables for E2EE
    EVP_PKEY* client_key_pair;
    EVP_PKEY* server_pub_key;
    std::vector<unsigned char> shared_secret_;
    std::vector<unsigned char> iv_;
    unsigned char iv[EVP_MAX_IV_LENGTH];
public:
    bool infoIsLoaded;
    bool possibleContactsLoaded;

    Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller);
    ~Messenger();
    void initializeConnection();
    void sendMessage(const std::string& receiver, const std::string& message);
    void logout();
    void setReceiveMessageCallback(std::function<void(Caller*, const QString&, unsigned long long)> callback);
    std::vector<Contact>& getFriendList();
    void requestChatHistory(long long id);
    std::vector<std::map<std::string, QString>>& getChatHistory();
    void tryFindByLogin(const QString& login);
    std::vector<Contact>& getPossibleContacts();
    void cleanPossibleContacts();
};

template <typename Caller>
Messenger<Caller>::Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller):
    service_{service},
    hash_{hash},
    caller_{caller},
    config_{"config.txt"},
    infoIsLoaded{true},
    possibleContactsLoaded{false}
{
    auto ip{ config_.getConfigValue("ipServer") };
    auto portStr{ config_.getConfigValue("portServer") };

    if (ip == std::nullopt || portStr == std::nullopt) {
        throw std::exception("Bad config values!");
    }

    port_ = std::stoi(portStr.value());
    ip_ = ip.value();

    handler_.reset(new ConnectionHandler<Messenger>{ service_, *this});
    handler_->setAsyncReadCallback(&Messenger::readCallback);
    handler_->setWriteCallback(&Messenger::writeCallback);
    key_exchange();
}
template <typename Caller>
Messenger<Caller>::~Messenger()
{
    handler_->getSocket().close();
}
template <typename Caller>
void Messenger<Caller>::readCallback(std::shared_ptr<IConnectionHandler<Messenger> > handler, const boost::system::error_code &err, size_t bytes_transferred)
{
    if(err){
        qDebug()<<err.message().c_str();
        return;
    }
    parseServerCommands(handler->getData());
    handler->resetStrBuf();
    handler->callAsyncRead();
}
template <typename Caller>
void Messenger<Caller>::writeCallback(std::shared_ptr<IConnectionHandler<Messenger> > handler, const boost::system::error_code &err, size_t bytes_transferred)
{
    if(err){
        qDebug()<< "ERROR: " << err.what().c_str() << " Bytes: " <<bytes_transferred;
    }
}
template <typename Caller>
void Messenger<Caller>::init(const boost::system::error_code &erCode)
{
    handler_->callWrite(hash_);
    handler_->callAsyncRead();
}
template <typename Caller>
void Messenger<Caller>::parseServerCommands(const std::string &data)
{
    Json::Value value;
    Json::Reader reader;
    reader.parse(data, value);
    switch(value["command"].asInt()){
    case SENDMESSAGE:
        receiveMessage(data);
        break;
    case FRIENDLIST:
        fillFriendList(data);
        break;
    case GETCHAT:
        fillChatHistory(data);
        break;
    case TRY_GET_CONTACT_BY_LOGIN:
        parsePossibleContacts(data);
        break;
    }

}
template <typename Caller>
void Messenger<Caller>::receiveMessage(const std::string &data)
{
    Json::Value value;
    Json::Reader reader;
    reader.parse(data, value);
    senderCallback_(caller_, decrypt_message(value["message"].asString()).c_str(), std::stoull(value["sender"].asString()));
}

template<typename Caller>
void Messenger<Caller>::fillFriendList(const std::string& jsonData)
{
    Json::Reader reader;
    Json::Value value;
    reader.parse(jsonData, value);
    auto dataArray{value["data"]};
    for(auto& dataValue : dataArray){
        QString lastMessage{""};
        unsigned long long senderId{};
        if(!dataValue["lastMessage"]["message"].empty()){
            lastMessage = dataValue["lastMessage"]["message"].asCString();
            senderId = dataValue["lastMessage"]["sender"].asUInt64();
        }
        Contact tmpContact {dataValue["name"].asCString(), dataValue["id"].asUInt64(), {senderId, lastMessage}};
        friendList_.push_back(tmpContact);
    }
    infoIsLoaded = false;
}
template <typename Caller>
void Messenger<Caller>::initializeConnection()
{
    handler_->getSocket().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_), port_),std::bind(&Messenger::init,this->shared_from_this(),std::placeholders::_1));
}
template <typename Caller>
void Messenger<Caller>::sendMessage(const std::string &receiver, const std::string &message)
{
    Json::Value value;
    Json::FastWriter writer;
    value["receiver"] = receiver;
    auto encryptedMsg{encrypt_message(message)};
    auto decryptedMsg{decrypt_message(encryptedMsg)};;
    qDebug()<<"Encrypted: " << encryptedMsg.c_str() << "Decrypted: " << decryptedMsg.c_str();
    value["message"] = encryptedMsg;
    value["command"] = SENDMESSAGE;
    handler_->callWrite(writer.write(value));
}
template <typename Caller>
void Messenger<Caller>::logout()
{
    Json::Value value;
    Json::FastWriter writer;
    value["command"] = "logout";
    handler_->callWrite(writer.write(value));
}
template <typename Caller>
void Messenger<Caller>::setReceiveMessageCallback(std::function<void (Caller*, const QString &, unsigned long long)> callback)
{
    senderCallback_= callback;
}
template <typename Caller>
std::vector<Contact>& Messenger<Caller>::getFriendList(){
    return friendList_;
}

template <typename Caller>
void Messenger<Caller>::requestChatHistory(long long id){
    Json::Value value;
    Json::FastWriter writer;
    value["receiver"] = std::to_string(id);
    value["command"] = GETCHAT;
    handler_->callWrite(writer.write(value));
}

template <typename Caller>
void Messenger<Caller>::fillChatHistory(const std::string& jsonData){
    Json::Reader reader;
    Json::Value value;
    reader.parse(jsonData, value, false);
    chatHistoryVector_.clear();
    for(auto& valueArray : value["data"]){
        std::map<std::string, QString> tmpMap;
        for(auto& key : valueArray.getMemberNames()){
            tmpMap[key] = valueArray[key].asCString();
        }
        chatHistoryVector_.push_back(tmpMap);
    }
    infoIsLoaded = false;
}

template <typename Caller>
std::vector<std::map<std::string, QString>>& Messenger<Caller>::getChatHistory(){
    return chatHistoryVector_;
}

template <typename Caller>
std::vector<Contact>& Messenger<Caller>::getPossibleContacts(){
    return possibleContactsVector_;
}

template <typename Caller>
void Messenger<Caller>::tryFindByLogin(const QString& login){
    Json::Value value;
    Json::FastWriter writer;
    value["command"] = TRY_GET_CONTACT_BY_LOGIN;
    value["LOGIN"] = login.toStdString();
    handler_->callWrite(writer.write(value));
}

template <typename Caller>
void Messenger<Caller>::parsePossibleContacts(const std::string& jsonData){
    Json::Reader reader;
    Json::Value value;
    reader.parse(jsonData, value);
    auto dataArray{value["data"]};
    for(auto& dataValue : dataArray){
        QString login{""};
        unsigned long long id{};
        login = dataValue["LOGIN"].asCString();
        id = dataValue["ID"].asUInt64();
        Contact tmpContact {login, id, {1, ""}};
        possibleContactsVector_.push_back(tmpContact);
    }
    possibleContactsLoaded = true;
}

template <typename Caller>
void Messenger<Caller>::cleanPossibleContacts(){
    possibleContactsVector_.clear();
}


template <typename Caller>
void Messenger<Caller>::key_exchange() {
    // 1. Generate an RSA key pair for the client
    client_key_pair = EVP_PKEY_new();
    RSA* rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    EVP_PKEY_set1_RSA(client_key_pair, rsa);

    // 2. Send client's public key to the server and receive the server's public key
    // Note: You need to modify your server-side code to support key exchange
    // For demonstration purposes, we assume you have a function called 'exchange_public_keys'
    server_pub_key = send_and_receive_public_keys(client_key_pair);

    // 3. Derive a shared secret using the client's private key and the server's public key
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(client_key_pair, nullptr);
    EVP_PKEY_derive_init(ctx);
    EVP_PKEY_derive_set_peer(ctx, server_pub_key);
    size_t secret_len = 32;
    EVP_PKEY_derive(ctx, nullptr, &secret_len);
    shared_secret_.resize(secret_len);
    EVP_PKEY_derive(ctx, shared_secret_.data(), &secret_len);
    EVP_PKEY_CTX_free(ctx);

    // Clean up
    RSA_free(rsa);
}

template <typename Caller>
std::string Messenger<Caller>::encrypt_message(const std::string& message) {
    // 1. Generate a random IV for encryption
    iv_.resize(16);
    RAND_bytes(iv_.data(), iv_.size());

    // 2. Create an AES-256 cipher context using the shared secret and IV
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, shared_secret_.data(), iv_.data());

    // 3. Encrypt the plaintext message using the cipher context
    int encrypted_len = message.length() + EVP_MAX_BLOCK_LENGTH;
    unsigned char* encrypted = new unsigned char[encrypted_len];
    int final_len;
    EVP_EncryptUpdate(ctx, encrypted, &encrypted_len, reinterpret_cast<const unsigned char*>(message.data()), message.length());
    EVP_EncryptFinal_ex(ctx, encrypted + encrypted_len, &final_len);
    encrypted_len += final_len;
    EVP_CIPHER_CTX_free(ctx);

    // 4. Combine the IV and encrypted message
    std::vector<unsigned char> combined(iv_.begin(), iv_.end());
    combined.insert(combined.end(), encrypted, encrypted + encrypted_len);

    // 5. Encode the combined message using Base64
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);
    BIO_write(b64, combined.data(), combined.size());
    BIO_flush(b64);

    BUF_MEM* buffer;
    BIO_get_mem_ptr(b64, &buffer);

    std::string encoded(buffer->data, buffer->length);

    // Clean up
    delete[] encrypted;
    BIO_free_all(b64);

    return encoded;
}

template <typename Caller>
EVP_PKEY* Messenger<Caller>::send_and_receive_public_keys(EVP_PKEY* client_public_key) {
//    // Convert the client's public key to a PEM string
//    BIO* bio = BIO_new(BIO_s_mem());
//    PEM_write_bio_PUBKEY(bio, client_public_key);
//    char* pem_data = nullptr;
//    long pem_size = BIO_get_mem_data(bio, &pem_data);
//    std::string client_pub_key_str(pem_data, pem_size);
//    BIO_free(bio);

//    // Send the client's public key to the server
//    //send_message_to_server(client_pub_key_str); TODOD

//    // Receive the server's public key as a string
//    std::string server_pub_key_str = "Server Pub Key";//receive_message_from_server();

//    // Convert the server's public key string back to an EVP_PKEY
//    bio = BIO_new_mem_buf(server_pub_key_str.data(), server_pub_key_str.size());
//    EVP_PKEY* server_public_key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
//    BIO_free(bio);

//    return server_public_key;
    // Convert the client's public key to a PEM string
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, client_public_key);
    char* pem_data = nullptr;
    long pem_size = BIO_get_mem_data(bio, &pem_data);
    std::string client_pub_key_str(pem_data, pem_size);
    BIO_free(bio);

    // Simulate sending the client's public key to the server
    //send_message_to_server(client_pub_key_str);

    // Simulate the server generating an RSA key pair
    EVP_PKEY* server_key_pair = EVP_PKEY_new();
    RSA* server_rsa = RSA_generate_key(2048, RSA_F4, nullptr, nullptr);
    EVP_PKEY_set1_RSA(server_key_pair, server_rsa);

    // Extract the server's public key from its key pair
    RSA* server_pub_rsa = RSAPublicKey_dup(EVP_PKEY_get1_RSA(server_key_pair));
    EVP_PKEY* server_public_key = EVP_PKEY_new();
    EVP_PKEY_set1_RSA(server_public_key, server_pub_rsa);

    // Simulate receiving the server's public key as a string
    //std::string server_pub_key_str = "Server Pub Key";//receive_message_from_server();
    bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bio, server_public_key);
    pem_data = nullptr;
    pem_size = BIO_get_mem_data(bio, &pem_data);
    std::string server_pub_key_str(pem_data, pem_size);
    BIO_free(bio);

    // Convert the server's public key string back to an EVP_PKEY
    bio = BIO_new_mem_buf(server_pub_key_str.data(), server_pub_key_str.size());
    EVP_PKEY* received_server_public_key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    // Clean up
    RSA_free(server_rsa);
    RSA_free(server_pub_rsa);
    EVP_PKEY_free(server_key_pair);

    return received_server_public_key;
}

template <typename Caller>
std::string Messenger<Caller>::decrypt_message(const std::string& encrypted_message) {
    // Decode the Base64-encoded encrypted message
    BIO* bio = BIO_new_mem_buf(encrypted_message.data(), encrypted_message.size());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);

    std::vector<unsigned char> decoded_message(encrypted_message.size());
    int decoded_len = BIO_read(b64, decoded_message.data(), decoded_message.size());
    BIO_free_all(b64);

    // Separate the IV and encrypted message
    std::vector<unsigned char> iv(decoded_message.begin(), decoded_message.begin() + 16);
    std::vector<unsigned char> encrypted_message_binary(decoded_message.begin() + 16, decoded_message.begin() + decoded_len);

    // Initialize the context for decryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context for decryption");
    }

    // Set the decryption key and IV
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, shared_secret_.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set decryption key and IV");
    }

    // Decrypt the message
    std::vector<unsigned char> decrypted_message(encrypted_message_binary.size());
    int decrypted_len = 0;

    if (!EVP_DecryptUpdate(ctx, decrypted_message.data(), &decrypted_len, encrypted_message_binary.data(), encrypted_message_binary.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt message");
    }

    int final_len = 0;
    decrypted_message.resize(decrypted_len + EVP_MAX_BLOCK_LENGTH);
    if (!EVP_DecryptFinal_ex(ctx, decrypted_message.data() + decrypted_len, &final_len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize decryption");
    }

    decrypted_message.resize(decrypted_len + final_len);
    EVP_CIPHER_CTX_free(ctx);

    return std::string(decrypted_message.begin(), decrypted_message.end());
}

