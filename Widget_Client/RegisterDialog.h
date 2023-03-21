#pragma once

#include <QDialog>
#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <mutex>
#include <condition_variable>
#include "Config.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog, public std::enable_shared_from_this<RegisterDialog>
{
    Q_OBJECT

    std::string ip_;
    short port_;
    boost::asio::io_service& service_;
    std::shared_ptr<IConnectionHandler<RegisterDialog>> handler_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::string serverResponseString_;
    std::string hash_;
    Config config_;

    void writeCallback(std::shared_ptr<IConnectionHandler<RegisterDialog>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void readCallback(std::shared_ptr<IConnectionHandler<RegisterDialog>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    std::string createDeviceId();
    unsigned int checkServerResponse();
public:
    explicit RegisterDialog(boost::asio::io_service& service, QWidget *parent = nullptr);
    ~RegisterDialog();

    unsigned int checkLoginData();
    void initializeConnection();
    std::string getHash();
private slots:
    void on_LoginButton_clicked();

    void on_RegisterButton_clicked();

private:
    Ui::RegisterDialog *ui;
    bool validateCredentials();
    void sendCredentials(const std::string& command);

signals:
    void onSuccessfulLogin(const std::string& hash);
};

