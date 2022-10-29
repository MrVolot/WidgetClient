#pragma once
#include <boost/asio.hpp>

class IoServiceWorker
{
    std::unique_ptr<boost::asio::io_service> service_;
    std::unique_ptr<boost::asio::io_service::work> serviceWork_;
    std::unique_ptr<std::thread> workThread_;
    IoServiceWorker();
public:
    ~IoServiceWorker();

    static IoServiceWorker& getInstance();
    boost::asio::io_service& getService();
};
