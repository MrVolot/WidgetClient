#pragma once

#include <QDialog>
#include <boost/asio.hpp>
#include "../ConnectionHandler/headers/ConnectionHandler.h"
#include <mutex>
#include <condition_variable>
#include "Config.h"
#include <boost/asio/ssl.hpp>
#include "../ConnectionHandler/headers/ConnectionHandlerSsl.h"


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
    boost::asio::ssl::context ssl_context_;

    void writeCallback(std::shared_ptr<IConnectionHandler<RegisterDialog>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void readCallback(std::shared_ptr<IConnectionHandler<RegisterDialog>> handler, const boost::system::error_code &err, size_t bytes_transferred);
    void init(const boost::system::error_code& erCode);
    std::string createDeviceId();
    unsigned int checkServerResponse();
    bool custom_verify_callback(bool preverified, boost::asio::ssl::verify_context& ctx);

    std::pair<boost::asio::const_buffer, boost::asio::const_buffer> generate_self_signed_certificate_and_key() {
        // Generate key pair
        EVP_PKEY* pkey = EVP_PKEY_new();
        RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
        EVP_PKEY_assign_RSA(pkey, rsa);

        // Generate self-signed certificate
        X509* cert = X509_new();
        ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);
        X509_gmtime_adj(X509_get_notBefore(cert), 0);
        X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);  // 1 year
        X509_set_pubkey(cert, pkey);

        X509_NAME* name = X509_get_subject_name(cert);
        X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"qw", -1, -1, 0);
        X509_set_issuer_name(cert, name);

        X509_sign(cert, pkey, EVP_sha256());

        // Write the certificate and private key to memory BIOs
        BIO* cert_bio = BIO_new(BIO_s_mem());
        BIO* key_bio = BIO_new(BIO_s_mem());
        PEM_write_bio_X509(cert_bio, cert);
        PEM_write_bio_PrivateKey(key_bio, pkey, NULL, NULL, 0, NULL, NULL);

        // Convert memory BIOs to const_buffer
        BUF_MEM* cert_mem;
        BUF_MEM* key_mem;
        BIO_get_mem_ptr(cert_bio, &cert_mem);
        BIO_get_mem_ptr(key_bio, &key_mem);
        boost::asio::const_buffer cert_buffer(cert_mem->data, cert_mem->length);
        boost::asio::const_buffer key_buffer(key_mem->data, key_mem->length);

        // Cleanup
        X509_free(cert);
        EVP_PKEY_free(pkey);

        // Return the const_buffers
        return std::make_pair(cert_buffer, key_buffer);
    }
public:
    explicit RegisterDialog(boost::asio::io_service& service, QWidget *parent = nullptr);
    ~RegisterDialog();

    unsigned int checkLoginData();
    void initializeConnection();
    std::string getHash();
    void processAfterHandshake();
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

