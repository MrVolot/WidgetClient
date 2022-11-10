#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"

RegisterDialog::RegisterDialog(boost::asio::io_service& service, QWidget *parent) :
    QDialog(parent),
    service_{service},
    config_{"config.txt"},
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    auto ip{ config_.getConfigValue("ipLoginServer") };
    auto portStr{ config_.getConfigValue("portLoginServer") };

    if (ip == std::nullopt || portStr == std::nullopt) {
        throw std::exception("Bad config values!");
    }

    port_ = std::stoi(portStr.value());
    ip_ = ip.value();

    handler_.reset(new ConnectionHandler<RegisterDialog>{ service_, *this});
    handler_->setAsyncReadCallback(&RegisterDialog::readCallback);
    handler_->setWriteCallback(&RegisterDialog::writeCallback);
}

RegisterDialog::~RegisterDialog()
{
    handler_->getSocket().close();
    delete ui;
}

void RegisterDialog::writeCallback(std::shared_ptr<IConnectionHandler<RegisterDialog> > handler, const boost::system::error_code &err, size_t bytes_transferred)
{
    if(err){
       qDebug()<<err.what().c_str();
    }
}

void RegisterDialog::readCallback(std::shared_ptr<IConnectionHandler<RegisterDialog> > handler, const boost::system::error_code &err, size_t bytes_transferred)
{
    serverResponseString_.erase();
    serverResponseString_ = handler->getData();
    cv_.notify_all();
}

void RegisterDialog::init(const boost::system::error_code& erCode)
{
    if(erCode){
        qDebug()<<erCode.message().c_str();
    }
}

void RegisterDialog::initializeConnection()
{
    handler_->getSocket().async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_), port_),std::bind(&RegisterDialog::init,shared_from_this(),std::placeholders::_1));
}

std::string RegisterDialog::getHash()
{
    return hash_;
}

unsigned int RegisterDialog::checkServerResponse()
{
    Json::Value value;
    Json::Reader reader;
    reader.parse(serverResponseString_, value);
    if(!value["token"].empty()){
        hash_ = value["token"].asString();
    }
    return value["command"].asInt();
}

unsigned int RegisterDialog::checkLoginData()
{
    Json::Value value;
    Json::FastWriter writer;
    value["command"] = "auth";
    value["deviceId"] = createDeviceId();
    handler_->callWrite(writer.write(value));
    handler_->callAsyncRead();
    std::unique_lock<std::mutex> locker{mtx_};
    cv_.wait(locker);
    return checkServerResponse();
}

std::string RegisterDialog::createDeviceId()
{
    std::string id{QSysInfo::currentCpuArchitecture().toLocal8Bit().constData()};
    id.append(QSysInfo::prettyProductName().toLocal8Bit().constData() );
    id.append(QSysInfo::kernelType().toLocal8Bit().constData());
    id.append(QSysInfo::kernelVersion().toLocal8Bit().constData());
    id.append(QSysInfo::machineHostName().toLocal8Bit().constData());
    return id;
}

void RegisterDialog::on_LoginButton_clicked()
{
    Json::Value value;
    Json::FastWriter writer_;
    value["command"] = "login";
    auto lol = ui->Login->text().toStdString();
    value["login"] = ui->Login->text().toStdString();
    value["deviceId"] = createDeviceId();
    value["password"] = ui->Password->text().toStdString();
    handler_->callWrite(writer_.write(value));
    handler_->callAsyncRead();
    std::unique_lock<std::mutex> locker{mtx_};
    cv_.wait(locker);
    auto serverResponse {checkServerResponse()};
    if(serverResponse == 0x0001 || serverResponse == 0x0002){
        emit onSuccessfulLogin(hash_);
    }
    if(serverResponse == 0x0004){
        ui->Password->clear();
        ui->Password->setPlaceholderText("Wrong credentials!");
    }
}


void RegisterDialog::on_RegisterButton_clicked()
{
    Json::Value value;
    Json::FastWriter writer_;
    value["command"] = "register";
    auto lol = ui->Login->text().toStdString();
    value["login"] = ui->Login->text().toStdString();
    value["deviceId"] = createDeviceId();
    value["password"] = ui->Password->text().toStdString();
    handler_->callWrite(writer_.write(value));
    handler_->callAsyncRead();
    std::unique_lock<std::mutex> locker{mtx_};
    cv_.wait(locker);
    auto serverResponse {checkServerResponse()};
    if(serverResponse == 0x0001 || serverResponse == 0x0002){
        emit onSuccessfulLogin(hash_);
    }
    if(serverResponse == 0x0005){
        ui->Password->clear();
        ui->Password->setPlaceholderText("User already exists!");
    }
}

