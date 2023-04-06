#pragma once

#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <Contact.h>
#include "../Json/json/json.h"
#include <Commands.h>
#include <ContactsWidget.h>
#include <iostream>
#include "Config.h"
#include "SecureTransmitter.h"

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
    std::function<void(Caller*, const QString&, unsigned long long)> senderCallback_;

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
public:
    bool infoIsLoaded;
    bool possibleContactsLoaded;

    Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller);
    ~Messenger();
    void initializeConnection();
    void sendMessage(unsigned long long receiverId, const std::string& message);
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

    secureTransmitter_.reset(new SecureTransmitter{});
    secureTransmitter_->setPrivateKey(privateKeyLocation.value());
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
    auto receiverId{std::stoull(value["sender"].asString())};
    auto sharedKey {tryGetSharedKeyById(receiverId)};
    if(sharedKey.has_value()){
        senderCallback_(caller_, secureTransmitter_->decryptMessage(value["message"].asString(), sharedKey.value()).c_str(), receiverId);
    }
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
void Messenger<Caller>::sendMessage(unsigned long long receiverId, const std::string &message)
{
    Json::Value value;
    Json::FastWriter writer;
    value["receiver"] = std::to_string(receiverId);//maybe leave without converting to string?
    auto sharedKey {tryGetSharedKeyById(receiverId)};
    if(sharedKey.has_value()){
        auto encryptedMsg{secureTransmitter_->encryptMessage(message, sharedKey.value())};
        value["message"] = encryptedMsg;
        value["command"] = SENDMESSAGE;
        handler_->callWrite(writer.write(value));
    }
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
