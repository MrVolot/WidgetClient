#pragma once

#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <Contact.h>
#include "../Json/json/json.h"
#include <Commands.h>
#include <ContactsWidget.h>
#include <iostream>
#include "Config.h"


// Include necessary OpenSSL headers
#pragma warning(disable : 4996)
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "openssl/err.h"

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

    // Necessary member variables for E2EE
    EVP_PKEY* privateKey;

    std::function<void(Caller*, const QString&, unsigned long long)> senderCallback_;

    void readCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void writeCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    void parseServerCommands(const std::string& data);
    void receiveMessage(const std::string& data);
    void fillFriendList(const std::string& jsonData);
    void fillChatHistory(const std::string& jsonData);
    void parsePossibleContacts(const std::string& jsonData);

    Contact getContactById(long long id){
        for (auto& contact : friendList_) {
            if (id == contact.getId()) {
                return contact;
            }
        }
    }

    void print_openssl_error() {
        unsigned long err = ERR_get_error();
        if (err != 0) {
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            qDebug() << "OpenSSL error: " << err_buf ;
        } else {
            std::cerr << "No OpenSSL error available" << std::endl;
        }
    }
    //E2EE methods
    std::vector<unsigned char> generateSharedKey(const std::string& userPublicKey);
    void setPrivateKey(const std::string& private_key_file);
    std::string encryptMessage(const std::string& message, long long receiverId);
    std::string decryptMessage(const std::string& encrypted_message, long long receiverId);
    EVP_PKEY* transformStringKey(const std::string& public_key_str);
public:
    bool infoIsLoaded;
    bool possibleContactsLoaded;

    Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller);
    ~Messenger();
    void initializeConnection();
    void sendMessage(long long receiverId, const std::string& message);
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
    OpenSSL_add_all_algorithms();
    auto ip{ config_.getConfigValue("ipServer") };
    auto portStr{ config_.getConfigValue("portServer") };
    auto privateKeyLocation{ config_.getConfigValue("privateKeyLocation") };

    if (ip == std::nullopt || portStr == std::nullopt || privateKeyLocation == std::nullopt) {
        throw std::exception("Bad config values!");
    }

    port_ = std::stoi(portStr.value());
    ip_ = ip.value();

    handler_.reset(new ConnectionHandler<Messenger>{ service_, *this});
    handler_->setAsyncReadCallback(&Messenger::readCallback);
    handler_->setWriteCallback(&Messenger::writeCallback);
    setPrivateKey(privateKeyLocation.value());
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
    senderCallback_(caller_, decryptMessage(value["message"].asString(), std::stoull(value["sender"].asString())).c_str(), std::stoull(value["sender"].asString()));
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
        std::string publicKey{dataValue["publicKey"].asString()};
        if(!dataValue["lastMessage"]["message"].empty()){
            lastMessage = dataValue["lastMessage"]["message"].asCString();
            senderId = dataValue["lastMessage"]["sender"].asUInt64();
        }
        std::vector<unsigned char> sharedKey{};
        if(publicKey.length()>4){
            qDebug()<<"generate was called";
            sharedKey = generateSharedKey(publicKey);
        }
        Contact tmpContact {dataValue["name"].asCString(), dataValue["id"].asUInt64(), {senderId, lastMessage}, sharedKey};
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
void Messenger<Caller>::sendMessage(long long receiverId, const std::string &message)
{
    Json::Value value;
    Json::FastWriter writer;
    value["receiver"] = std::to_string(receiverId);//maybe leave without converting to string?
    auto encryptedMsg{encryptMessage(message, receiverId)};
    auto decryptedMsg{decryptMessage(encryptedMsg, receiverId)};
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
std::vector<unsigned char> Messenger<Caller>::generateSharedKey(const std::string& userPublicKey) {
    EVP_PKEY* user_pub_key = transformStringKey(userPublicKey);

    // Derive a shared secret using the client's private key and the server's public key
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }

    if (EVP_PKEY_derive_init(ctx) != 1) {
        print_openssl_error();
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize key derivation");
    }

    if (EVP_PKEY_derive_set_peer(ctx, user_pub_key) != 1) {
        EVP_PKEY_CTX_free(ctx);
        print_openssl_error();
        throw std::runtime_error("Failed to set peer key");
    }

    size_t secret_len = 32;
    std::vector<unsigned char> shared_secretTMP(secret_len);
    if (EVP_PKEY_derive(ctx, shared_secretTMP.data(), &secret_len) != 1) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to derive shared secret");
    }

    EVP_PKEY_CTX_free(ctx);

    // Clean up
    EVP_PKEY_free(user_pub_key);

    std::string str(shared_secretTMP.begin(), shared_secretTMP.end());
    qDebug() << str.c_str();
    return shared_secretTMP;
}

template <typename Caller>
std::string Messenger<Caller>::encryptMessage(const std::string& message, long long receiverId) {
    // 1. Generate a random IV for encryption
    std::vector<unsigned char> iv;
    iv.resize(16);
    RAND_bytes(iv.data(), iv.size());

    // 2. Create an AES-256 cipher context using the shared secret and IV
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    auto currentContact{getContactById(receiverId)};
    auto sharedKey {currentContact.getSharedKey()};
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, sharedKey.data(), iv.data());

    // 3. Encrypt the plaintext message using the cipher context
    int decryptedLen = message.length() + EVP_MAX_BLOCK_LENGTH;
    unsigned char* encrypted = new unsigned char[decryptedLen];
    int finalLen;
    EVP_EncryptUpdate(ctx, encrypted, &decryptedLen, reinterpret_cast<const unsigned char*>(message.data()), message.length());
    EVP_EncryptFinal_ex(ctx, encrypted + decryptedLen, &finalLen);
    decryptedLen += finalLen;
    EVP_CIPHER_CTX_free(ctx);

    // 4. Combine the IV and encrypted message
    std::vector<unsigned char> combined(iv.begin(), iv.end());
    combined.insert(combined.end(), encrypted, encrypted + decryptedLen);

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
EVP_PKEY* Messenger<Caller>::transformStringKey(const std::string& publicKeyStr) {
    // Create a new BIO memory buffer and load the public key string into it
    BIO* publicKeyBio = BIO_new_mem_buf(publicKeyStr.data(), static_cast<int>(publicKeyStr.size()));
    if (!publicKeyBio) {
        throw std::runtime_error("Failed to create BIO memory buffer");
    }

    // Read the public key from the BIO memory buffer
    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(publicKeyBio, nullptr, nullptr, nullptr);
    BIO_free(publicKeyBio);

    if (!publicKey) {
        throw std::runtime_error("Failed to read server's public key");
    }

    return publicKey;
}

template <typename Caller>
std::string Messenger<Caller>::decryptMessage(const std::string& encryptedMessage, long long receiverId) {
    // Decode the Base64-encoded encrypted message
    BIO* bio = BIO_new_mem_buf(encryptedMessage.data(), encryptedMessage.size());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);

    std::vector<unsigned char> decodedMessage(encryptedMessage.size());
    int decodedLen = BIO_read(b64, decodedMessage.data(), decodedMessage.size());
    BIO_free_all(b64);

    // Separate the IV and encrypted message
    std::vector<unsigned char> iv(decodedMessage.begin(), decodedMessage.begin() + 16);
    std::vector<unsigned char> encryptedMessageBinary(decodedMessage.begin() + 16, decodedMessage.begin() + decodedLen);

    // Initialize the context for decryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context for decryption");
    }

    auto currentContact{getContactById(receiverId)};
    auto sharedKey {currentContact.getSharedKey()};

    // Set the decryption key and IV
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, sharedKey.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to set decryption key and IV");
    }

    // Decrypt the message
    std::vector<unsigned char> decryptedMessage(encryptedMessageBinary.size());
    int decryptedLen = 0;

    if (!EVP_DecryptUpdate(ctx, decryptedMessage.data(), &decryptedLen, encryptedMessageBinary.data(), encryptedMessageBinary.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt message");
    }

    int final_len = 0;
    decryptedMessage.resize(decryptedLen + EVP_MAX_BLOCK_LENGTH);
    if (!EVP_DecryptFinal_ex(ctx, decryptedMessage.data() + decryptedLen, &final_len)) {
        print_openssl_error();
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize decryption");
    }

    decryptedMessage.resize(decryptedLen + final_len);
    EVP_CIPHER_CTX_free(ctx);

    return std::string(decryptedMessage.begin(), decryptedMessage.end());
}

template <typename Caller>
void Messenger<Caller>::setPrivateKey(const std::string& privateKeyFilePath)
{
    // Read the client's private key from the file
    BIO* privateKeyBio = BIO_new_file(privateKeyFilePath.c_str(), "r");
    if (!privateKeyBio) {
        throw std::runtime_error("Failed to open private key file");
    }
    privateKey = PEM_read_bio_PrivateKey(privateKeyBio, nullptr, nullptr, nullptr);
    BIO_free(privateKeyBio);

    if (!privateKey) {
        throw std::runtime_error("Failed to read private key");
    }
}
