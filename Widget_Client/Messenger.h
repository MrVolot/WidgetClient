#pragma once

#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/HttpsConnectionHandler.h"
#include <Contact.h>
#include "../Json/json/json.h"
#include <Commands.h>
#include <ContactsWidget.h>
#include <iostream>
#include "Config.h"
#include "MessageInfo.h"
#include "MessengerSignalHandler.h"
#include "SecureTransmitter.h"
#include "certificateUtils.h"

template <typename Caller>
class Messenger: public std::enable_shared_from_this<Messenger<Caller>>
{
    std::string ip_;
    short port_;
    boost::asio::io_service& service_;
    std::shared_ptr<IConnectionHandler<Messenger>> handler_;
    std::string hash_;
    std::string name_;
    Caller* caller_;
    Config config_;
    unsigned long long id_;
    //remove this and store messages after they are loaded
    //or create some other logic
    unsigned long long currentFriendIdForChatHistory_ = 0;
    std::vector<Contact> friendList_;
    std::vector<std::map<std::string, QString>> chatHistoryVector_;
    std::vector<Contact> possibleContactsVector_;
    std::unique_ptr<SecureTransmitter> secureTransmitter_;
    std::function<void(Caller*, const MessageInfo &)> senderCallback_;
    boost::asio::ssl::context ssl_context_;
    bool isGuestAccount_;
    std::map<std::string, std::promise<std::string>>pendingPublicKeyPromises_;
    std::string publicKey_;

    void readCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void writeCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    void parseServerCommands(const std::string& data);
    void receiveMessage(const std::string& data);
    void fillFriendList(const std::string& jsonData);
    void fillChatHistory(const std::string& jsonData);
    void parsePossibleContacts(const std::string& jsonData);
    std::optional<Contact> getContactById(long long id);
    std::optional<std::vector<unsigned char>> tryGetSharedKeyById(unsigned long long id);
    void deleteGuestAccount();
    std::string requestPublicKey(unsigned long long userId);
public:
    bool infoIsLoaded;
    bool possibleContactsLoaded;
    MessengerSignalHandler signalHandler;

    Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller, bool isGuestAccount = false);
    ~Messenger();
    void initializeConnection();
    void sendMessage(const MessageInfo & messageInfo);
    void logout();
    void setReceiveMessageCallback(std::function<void(Caller*, const MessageInfo &)> callback);
    std::vector<Contact>& getFriendList();
    void requestChatHistory(long long id);
    std::vector<std::map<std::string, QString>>& getChatHistory();
    void tryFindByLogin(const QString& login);
    std::vector<Contact>& getPossibleContacts();
    void cleanPossibleContacts();
    void processAfterHandshake();
    void removeMessageFromDb(const MessageInfo & msgInfo);
};

template <typename Caller>
Messenger<Caller>::Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller, bool isGuestAccount):
    service_{service},
    hash_{hash},
    caller_{caller},
    config_{"config.txt"},
    infoIsLoaded{true},
    possibleContactsLoaded{false},
    ssl_context_{boost::asio::ssl::context::sslv23},
    isGuestAccount_{isGuestAccount}
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

    std::shared_ptr<EVP_PKEY> private_key = certificateUtils::generate_private_key(2048);
    std::shared_ptr<X509> certificate = certificateUtils::generate_self_signed_certificate("ServerClient", private_key.get(), 365);

    // Load the CA certificate into memory
    std::shared_ptr<X509> ca_cert = certificateUtils::load_ca_certificate();

    // Add the CA certificate to the SSL context
    X509_STORE* cert_store = SSL_CTX_get_cert_store(ssl_context_.native_handle());
    X509_STORE_add_cert(cert_store, ca_cert.get());

    // Use the generated private key and certificate for the SSL context
    ssl_context_.use_private_key(boost::asio::const_buffer(certificateUtils::private_key_to_pem(private_key.get()).data(), certificateUtils::private_key_to_pem(private_key.get()).size()), boost::asio::ssl::context::pem);
    ssl_context_.use_certificate(boost::asio::const_buffer(certificateUtils::certificate_to_pem(certificate.get()).data(), certificateUtils::certificate_to_pem(certificate.get()).size()), boost::asio::ssl::context::pem);
    ssl_context_.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context_.set_verify_callback(
        [](bool preverified, boost::asio::ssl::verify_context& ctx) {
            return certificateUtils::custom_verify_callback(preverified, ctx, "Server");
        });

    handler_ = std::make_shared<HttpsConnectionHandler<Messenger, ConnectionHandlerType::CLIENT>>(service_, *this, ssl_context_);
    handler_->setAsyncReadCallback(&Messenger::readCallback);
    handler_->setWriteCallback(&Messenger::writeCallback);

    secureTransmitter_.reset(new SecureTransmitter{});
    if(isGuestAccount){
        publicKey_ = secureTransmitter_->generateKeys();
    }else{
        secureTransmitter_->setPrivateKey(privateKeyLocation.value());
    }
}

template <typename Caller>
Messenger<Caller>::~Messenger()
{
    if(isGuestAccount_){
        deleteGuestAccount();
    }
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
    if(erCode){
        qDebug()<<erCode.message().c_str();
    } else {
        handler_->callAsyncHandshake();
    }
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
    case REQUEST_PUBLIC_KEY:{
        std::string userId = value["id"].asString();
        std::string publicKey = value["userPublicKey"].asString();

        // If there's a promise waiting for the public key, set the value and remove the promise from the map
        auto it = pendingPublicKeyPromises_.find(userId);
        if (it != pendingPublicKeyPromises_.end()) {
            it->second.set_value(publicKey);
            pendingPublicKeyPromises_.erase(it);
        }
        break;
    }
    case DELETE_MESSAGE:
        QString chatId{value["receiver"].asCString()};
        if(chatId.toULongLong() == id_){
            chatId = value["sender"].asCString();
        }
        emit signalHandler.deleteMessageRequest(chatId, value["messageGuid"].asCString());
        break;
    }
}

template <typename Caller>
void Messenger<Caller>::receiveMessage(const std::string &data)
{
    Json::Value value;
    Json::Reader reader;
    reader.parse(data, value);
    auto senderId{std::stoull(value["sender"].asString())};
    auto possibleSharedKey {tryGetSharedKeyById(senderId)};
    std::vector<unsigned char> sharedKey{};
    if(possibleSharedKey.has_value()){
        //what about isAuthor?
        //TODO: REMOVE ISAUTHOR
        sharedKey = possibleSharedKey.value();
    }else{
        std::string publicKey = requestPublicKey(senderId);
        sharedKey = secureTransmitter_->generateSharedKey(publicKey);
        friendList_.push_back({"", senderId, {senderId, ""}, sharedKey});
    }
    bool isAuthor{false};
    if(senderId == id_){
        isAuthor = true;
    }
    MessageInfo msgInfo{value["messageGuid"].asCString(),
                        senderId,
                        std::stoull(value["receiver"].asString()),
                        secureTransmitter_->decryptMessage(value["message"].asString(),
                                                           sharedKey).c_str(),
                        value["time"].asCString(),
                        isAuthor};
    senderCallback_(caller_, msgInfo);
    //else should ask the server for the public key and then generate shared key
}

template<typename Caller>
void Messenger<Caller>::fillFriendList(const std::string& jsonData)
{
    Json::Reader reader;
    Json::Value value;
    reader.parse(jsonData, value);
    auto dataArray{value["data"]};
    id_ = value["personalId"].asLargestUInt();
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
            sharedKey = secureTransmitter_->generateSharedKey(publicKey);
            lastMessage = secureTransmitter_->decryptMessage(dataValue["lastMessage"]["message"].asString(), sharedKey).c_str();
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
void Messenger<Caller>::sendMessage(const MessageInfo & messageInfo)
{
    Json::Value value;
    Json::FastWriter writer;
    value["receiver"] = std::to_string(messageInfo.friendId);//maybe leave without converting to string?
    auto sharedKey {tryGetSharedKeyById(messageInfo.friendId)};
    std::string encryptedMsg;
    if(sharedKey.has_value()){
        encryptedMsg = secureTransmitter_->encryptMessage(messageInfo.text.toStdString(), sharedKey.value());

    }else{
        std::string publicKey = requestPublicKey(messageInfo.friendId);
        sharedKey = secureTransmitter_->generateSharedKey(publicKey);
        friendList_.push_back({"", messageInfo.friendId, {messageInfo.friendId, ""}, sharedKey.value()});
        encryptedMsg = secureTransmitter_->encryptMessage(messageInfo.text.toStdString(), sharedKey.value());
    }
    value["message"] = encryptedMsg;
    value["command"] = SENDMESSAGE;
    value["messageGuid"] = messageInfo.messageGuid.toStdString();
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
void Messenger<Caller>::setReceiveMessageCallback(std::function<void (Caller*, const MessageInfo &)> callback)
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
    currentFriendIdForChatHistory_ = id;
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
            auto sharedKey {tryGetSharedKeyById(currentFriendIdForChatHistory_)};
            if(key == "message" && sharedKey.has_value()){
                tmpMap[key] = secureTransmitter_->decryptMessage(valueArray[key].asString(), sharedKey.value()).c_str();
            }else{
                tmpMap[key] = valueArray[key].asCString();
            }
        }
        chatHistoryVector_.push_back(tmpMap);
    }
    infoIsLoaded = false;
}

template <typename Caller>
std::vector<std::map<std::string, QString>>& Messenger<Caller>::getChatHistory(){
    currentFriendIdForChatHistory_ = 0;
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
std::optional<std::vector<unsigned char>> Messenger<Caller>::tryGetSharedKeyById(unsigned long long id)
{
    try{
        auto currentContact{getContactById(id)};
        if(currentContact.has_value()){
            auto sharedKey {currentContact.value().getSharedKey()};
            if(sharedKey.size() > 4){
                return sharedKey;
            }
        }
        return std::nullopt;
    }catch(...){
        return std::nullopt;
    }
}

template <typename Caller>
std::optional<Contact> Messenger<Caller>::getContactById(long long id){
    for (auto& contact : friendList_) {
        if (id == contact.getId()) {
            return contact;
        }
    }
    return std::nullopt;
}

template <typename Caller>
void Messenger<Caller>::processAfterHandshake() {
    Json::Value value;
    Json::FastWriter writer;
    value["hash"] = hash_;
    if(!publicKey_.empty()){
        value["publicKey"] = publicKey_;
    }
    handler_->callWrite(writer.write(value));
}

template <typename Caller>
void Messenger<Caller>::removeMessageFromDb(const MessageInfo & msgInfo)
{
    Json::Value value;
    Json::FastWriter writer;
    value["command"] = DELETE_MESSAGE;
    value["messageGuid"] = msgInfo.messageGuid.toStdString();
    value["receiverId"] = msgInfo.receiverId;
    value["senderId"] = msgInfo.senderId;
    handler_->callWrite(writer.write(value));
}

template <typename Caller>
void Messenger<Caller>::deleteGuestAccount(){
    Json::Value value;
    Json::FastWriter writer;
    value["command"] = DELETE_ACCOUNT;
    value["id"] = std::to_string(id_);
    handler_->callWrite(writer.write(value));
}
template <typename Caller>
std::string Messenger<Caller>::requestPublicKey(unsigned long long userId) {
    std::promise<std::string> promise;
    auto future = promise.get_future();

    // Create a JSON value with the request command and user ID
    Json::Value value;
    Json::StreamWriterBuilder writer;
    std::string strUserId{std::to_string(userId)};
    value["command"] = REQUEST_PUBLIC_KEY;
    value["id"] = strUserId;

    // Send the request to the server
    handler_->callWrite(Json::writeString(writer, value));

    // Store the promise in a map, using the userId as the key
    pendingPublicKeyPromises_[strUserId] = std::move(promise);

    // Wait for the response
    return future.get();
}

