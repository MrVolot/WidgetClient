#include "WindowManager.h"

WindowManager::WindowManager(boost::asio::io_service& service): service_{service}
{
    registerDialog_.reset(new RegisterDialog{service});
    registerDialog_->initializeConnection();
    registerDialog_->setModal(true);
    connect(&(*registerDialog_), &RegisterDialog::onSuccessfulLogin, this, &WindowManager::onSuccessfulLogin);
    registerDialog_->exec();
}

void WindowManager::onSuccessfulLogin(const std::string& hash, bool isGuestAccount)
{
    registerDialog_->hide();
    mainWindow_.reset(new MainWindow{service_, hash, isGuestAccount});
    mainWindow_->show();
}
