#include "OnlineStatusChecker.h"
#include <QDebug>
#include <thread>
#include <chrono>

OnlineStatusChecker::OnlineStatusChecker(boost::asio::io_service& service, std::condition_variable& cv):
    ip_{"127.0.0.1"},
    LoginServerPort_{10679},
    ServerPort_{10690},
    service_{service},
    timer_{service},
    notifier_{cv}
{

}

void OnlineStatusChecker::loginServerConnectionHandling(const boost::system::error_code &erCode)
{
    if(!loginServerConnection_){
        return;
    }
    if(erCode){
        qDebug()<<erCode.message().c_str();
        loginServerTimeout();
        qDebug()<<"Login Server Disconnected";
        loginServerStatus = false;
    }else{
        loginServerTimeout();
        qDebug()<<"Login Server Connected";
        loginServerStatus = true;
        notifier_.notify_all();
    }
}

void OnlineStatusChecker::serverConnectionHandling(const boost::system::error_code &erCode)
{
    if(!serverConnection_){
        return;
    }
    if(erCode){
        serverTimeout();
        qDebug()<<"Server Disconnected";
        serverStatus = false;
    }else{
        serverTimeout();
        qDebug()<<"Server Connected";
        serverStatus = true;
    }
}

OnlineStatusChecker &OnlineStatusChecker::getInstance(boost::asio::io_service& service, std::condition_variable& cv)
{
    static OnlineStatusChecker instance{service, cv};
    return instance;
}

void OnlineStatusChecker::startPingingLoginServer()
{
    //LoginServerHandler_.reset(new ConnectionHandlerSsl<OnlineStatusChecker>{ service_, *this});
    LoginServerHandler_->getSocket().lowest_layer().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), LoginServerPort_),std::bind(&OnlineStatusChecker::loginServerConnectionHandling, this,std::placeholders::_1));
}

void OnlineStatusChecker::startPingingServer()
{
    //ServerHandler_.reset(new ConnectionHandlerSsl<OnlineStatusChecker>{ service_, *this});
    ServerHandler_->getSocket().lowest_layer().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), ServerPort_),std::bind(&OnlineStatusChecker::serverConnectionHandling, this,std::placeholders::_1));
}

void OnlineStatusChecker::turnOffPinginLoginServer()
{
    loginServerConnection_ = false;
}

void OnlineStatusChecker::turnOnLoginServerPinger()
{
    loginServerConnection_ = true;
}

void OnlineStatusChecker::turnOffPinginServer()
{
    serverConnection_ = false;
}

void OnlineStatusChecker::turnOnServerPinger()
{
    serverConnection_ = true;
}

void OnlineStatusChecker::loginServerTimeout()
{
    timer_.expires_at(boost::posix_time::microsec_clock::universal_time() + boost::posix_time::seconds(5));
    timer_.async_wait(boost::bind(&OnlineStatusChecker::startPingingLoginServer, this));
}

void OnlineStatusChecker::serverTimeout()
{
    timer_.expires_at(boost::posix_time::microsec_clock::universal_time() + boost::posix_time::seconds(5));
    timer_.async_wait(boost::bind(&OnlineStatusChecker::startPingingServer, this));
}


