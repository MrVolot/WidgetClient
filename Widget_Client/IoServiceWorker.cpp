#include "IoServiceWorker.h"
#include <thread>
#include <boost/bind.hpp>

IoServiceWorker::IoServiceWorker(): service_{new boost::asio::io_service{}}, serviceWork_{new boost::asio::io_service::work{*service_}},
    workThread_{new std::thread{[this](){service_->run();}}}
{

}

IoServiceWorker::~IoServiceWorker()
{
    serviceWork_.release();
    service_.release();
    workThread_->join();
}

IoServiceWorker &IoServiceWorker::getInstance()
{
    static IoServiceWorker instance{};
    return instance;
}

boost::asio::io_service &IoServiceWorker::getService()
{
    return *service_;
}
