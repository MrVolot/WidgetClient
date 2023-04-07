#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"

RegisterDialog::RegisterDialog(boost::asio::io_service& service, QWidget *parent) :
    QDialog(parent),
    service_{service},
    config_{"config.txt"},
    ssl_context_{boost::asio::ssl::context::sslv23},
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

    ssl_context_.load_verify_file("C:\\Users\\Kiril\\Desktop\\testing\\ca.crt"); // Replace with the correct path
    auto [cert_buffer, key_buffer] = generate_self_signed_certificate_and_key();
    ssl_context_.use_certificate(cert_buffer, boost::asio::ssl::context::file_format::pem);
    ssl_context_.use_private_key(key_buffer, boost::asio::ssl::context::file_format::pem);
//    ssl_context_.use_certificate_chain_file("C:\\Users\\Kiril\\Desktop\\testing\\client.crt");
//    ssl_context_.use_private_key_file("C:\\Users\\Kiril\\Desktop\\testing\\client.key", boost::asio::ssl::context::pem);
    ssl_context_.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);
    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context_.set_verify_callback(boost::bind(&RegisterDialog::custom_verify_callback, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    handler_ = std::make_shared<HttpsConnectionHandler<RegisterDialog>>(service_, *this, ssl_context_);

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
    if(err){
        handler_->getSocket().close();
        return;
    }
    serverResponseString_.erase();
    serverResponseString_ = handler->getData();
    handler->resetStrBuf();
    handler->callAsyncRead();
    cv_.notify_all();
}

void RegisterDialog::init(const boost::system::error_code& erCode)
{
    if(erCode){
        qDebug()<<erCode.message().c_str();
    } else {
        qDebug() << "Connected.";
        handler_->callAsyncHandshake();
        //handler_->callAsyncRead();
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

void RegisterDialog::processAfterHandshake()
{

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
    if(!validateCredentials()){
        return;
    }
    sendCredentials("login");
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
    if(!validateCredentials()){
        return;
    }
    sendCredentials("register");
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

bool RegisterDialog::validateCredentials()
{
//    if(ui->Login->text().length() > 16){
//        ui->Password->clear();
//        ui->Login->clear();
//        ui->Login->setPlaceholderText("Login is too long!");
//        return false;
//    }
//    if(ui->Login->text().length() < 4){
//        ui->Password->clear();
//        ui->Login->clear();
//        ui->Login->setPlaceholderText("Login is too short!");
//        return false;
//    }
//    if(ui->Password->text().length() < 8){
//        ui->Password->clear();
//        ui->Login->clear();
//        ui->Password->setPlaceholderText("Password is too short!");
//        return false;
//    }
    return true;
}

void RegisterDialog::sendCredentials(const std::string& command)
{
    Json::Value value;
    Json::FastWriter writer_;
    value["command"] = command;
    value["login"] = ui->Login->text().toStdString();
    value["deviceId"] = createDeviceId();
    value["password"] = ui->Password->text().toStdString();
    handler_->callWrite(writer_.write(value));
}

bool RegisterDialog::custom_verify_callback(bool preverified, boost::asio::ssl::verify_context& ctx) {
     // You can implement your custom verification logic here.
     // For now, we'll just return the value of 'preverified'.
    // Get the X509_STORE_CTX object
    X509_STORE_CTX* store_ctx = ctx.native_handle();

    // Get the current certificate and its depth in the chain
    int depth = X509_STORE_CTX_get_error_depth(store_ctx);
    X509* cert = X509_STORE_CTX_get_current_cert(store_ctx);

    // Convert the X509 certificate to a human-readable format
    BIO* bio = BIO_new(BIO_s_mem());
    X509_print(bio, cert);
    BUF_MEM* mem;
    BIO_get_mem_ptr(bio, &mem);
    std::string cert_info(mem->data, mem->length);
    BIO_free(bio);

    qDebug() << "Certificate depth: " << depth;
    qDebug() << "Certificate information: \n"<< cert_info.c_str() ;
    qDebug() << "Preverified: " << preverified;
     return true;
}

