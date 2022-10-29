#pragma once
#include "RegisterDialog.h"
#include "Mainwindow.h"

class WindowManager: public QObject
{
    Q_OBJECT

    std::shared_ptr<RegisterDialog> registerDialog_;
    std::shared_ptr<MainWindow> mainWindow_;
    //std::unique_ptr<QGuiApplication> app_;
    boost::asio::io_service& service_;
public:
    WindowManager(boost::asio::io_service& service);
private slots:
    void onSuccessfulLogin(const std::string& hash);
};

