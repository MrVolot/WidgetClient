#pragma once
#include "../ConnectionHandler/headers/IConnectionHandler.h"
#include <string>

class OnlineStatusChecker
{
    std::string ip_;
    short LoginServerPort_;
    short ServerPort_;
    boost::asio::io_service& service_;
    std::shared_ptr<IConnectionHandler<OnlineStatusChecker>> LoginServerHandler_;
    std::shared_ptr<IConnectionHandler<OnlineStatusChecker>> ServerHandler_;
    boost::asio::deadline_timer timer_;
    std::condition_variable& notifier_;
    bool loginServerConnection_{true};
    bool serverConnection_{true};

    OnlineStatusChecker(boost::asio::io_service& service, std::condition_variable& cv);
    void loginServerConnectionHandling(const boost::system::error_code& erCode);
    void serverConnectionHandling(const boost::system::error_code& erCode);
    void loginServerTimeout();
    void serverTimeout();
public:
    bool loginServerStatus;
    bool serverStatus;
    static OnlineStatusChecker& getInstance(boost::asio::io_service& service, std::condition_variable& cv);
    void startPingingLoginServer();
    void startPingingServer();
    void turnOffPinginLoginServer();
    void turnOnLoginServerPinger();
    void turnOffPinginServer();
    void turnOnServerPinger();
};
