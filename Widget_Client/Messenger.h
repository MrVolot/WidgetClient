#pragma once

#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <Contact.h>
#include "../Json/json/json.h"
#include <Commands.h>
#include <ContactsWidget.h>
#include "Config.h"

template <typename Caller>
class Messenger: public std::enable_shared_from_this<Messenger<Caller>>
{
    std::string ip_;
    short port_;
    boost::asio::io_service& service_;
    std::shared_ptr<IConnectionHandler<Messenger>> handler_;
    std::unordered_map<long long, Contact> contactMap_;
    long long userId_;
    std::string hash_;
    std::string name_;
    Caller* caller_;
    Config config_;
    std::vector<Contact> friendList_;


    std::function<void(Caller*, const std::string& sender, const std::string& message)> senderCallback_;

    void readCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void writeCallback(std::shared_ptr<IConnectionHandler<Messenger>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    void parseServerCommands(const std::string& data);
    void receiveMessage(const std::string& data);
    void fillFriendList(const std::string& jsonData);
public:
    bool infoIsLoaded;

    Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller);
    ~Messenger();
    void initializeConnection();
    void sendMessage(const std::string& receiver, const std::string& message);
    void logout();
    void setReceiveMessageCallback(std::function<void(Caller*, const std::string& sender, const std::string& message)> callback);
    std::vector<Contact>& getFriendList();
};

template <typename Caller>
Messenger<Caller>::Messenger(boost::asio::io_service& service, const std::string& hash, Caller* caller):
    service_{service},
    hash_{hash},
    caller_{caller},
    config_{"config.txt"},
    infoIsLoaded{true}
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
    auto data{boost::asio::buffer_cast<const char*>(handler->getStrBuf()->data())};
    parseServerCommands(data);
    handler_->callAsyncRead();
}
template <typename Caller>
void Messenger<Caller>::writeCallback(std::shared_ptr<IConnectionHandler<Messenger> > handler, const boost::system::error_code &err, size_t bytes_transferred)
{
    qDebug()<< "ERROR: " << err.what().c_str() << " Bytes: " <<bytes_transferred;
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
    }

}
template <typename Caller>
void Messenger<Caller>::receiveMessage(const std::string &data)
{
    Json::Value value;
    Json::Reader reader;
    reader.parse(data, value);
    qDebug()<<value["sender"].asCString() << value["message"].asCString();
    //senderCallback_(caller_, value["sender"].asString(), value["message"].asString());
}

template<typename Caller>
void Messenger<Caller>::fillFriendList(const std::string& jsonData)
{
    Json::Reader reader;
    Json::Value value;
    reader.parse(jsonData, value);
    for(auto& key : value.getMemberNames()){
        if(value[key.c_str()].isString()){
            Contact tmpContact {value[key.c_str()].asCString(), std::stoi(key.c_str())};
            friendList_.push_back(tmpContact);
        }
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
    value["message"] = message;
    value["command"] = "sendMessage";
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
void Messenger<Caller>::setReceiveMessageCallback(std::function<void (Caller*, const std::string &, const std::string &)> callback)
{
    senderCallback_= callback;
}
template <typename Caller>
std::vector<Contact>& Messenger<Caller>::getFriendList(){
    return friendList_;
}
