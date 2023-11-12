#include "SecureTransmitter.h"
#include <ostream>
#include <stdexcept>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include "openssl/err.h"
#include <QDebug>

SecureTransmitter::SecureTransmitter(): privateKey_{nullptr}
{

}

std::vector<unsigned char> SecureTransmitter::generateSharedKey(const std::string& userPublicKey) {
    EVP_PKEY* user_pub_key = transformStringKey(userPublicKey);

    // Derive a shared secret using the client's private key and the server's public key
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey_, nullptr);
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_PKEY_CTX");
    }

    if (EVP_PKEY_derive_init(ctx) != 1) {
        print_openssl_error();
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize key derivation");
    }

    if (EVP_PKEY_derive_set_peer(ctx, user_pub_key) != 1) {
        EVP_PKEY_CTX_free(ctx);
        print_openssl_error();
        throw std::runtime_error("Failed to set peer key");
    }

    size_t secret_len = 32;
    std::vector<unsigned char> shared_secretTMP(secret_len);
    if (EVP_PKEY_derive(ctx, shared_secretTMP.data(), &secret_len) != 1) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error("Failed to derive shared secret");
    }

    EVP_PKEY_CTX_free(ctx);

    // Clean up
    EVP_PKEY_free(user_pub_key);

    std::string str(shared_secretTMP.begin(), shared_secretTMP.end());
    return shared_secretTMP;
}

std::string SecureTransmitter::encryptMessage(const std::string& message, std::vector<unsigned char> sharedKey) {
    // 1. Generate a random IV for encryption
    std::vector<unsigned char> iv;
    iv.resize(16);
    RAND_bytes(iv.data(), static_cast<int>(iv.size()));

    // 2. Create an AES-256 cipher context using the shared secret and IV
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, sharedKey.data(), iv.data());

    // 3. Encrypt the plaintext message using the cipher context
    auto convertedLength{static_cast<int>(message.length())};
    int decryptedLen = convertedLength + EVP_MAX_BLOCK_LENGTH;
    unsigned char* encrypted = new unsigned char[decryptedLen];
    int finalLen;
    EVP_EncryptUpdate(ctx, encrypted, &decryptedLen, reinterpret_cast<const unsigned char*>(message.data()), convertedLength);
    EVP_EncryptFinal_ex(ctx, encrypted + decryptedLen, &finalLen);
    decryptedLen += finalLen;
    EVP_CIPHER_CTX_free(ctx);

    // 4. Combine the IV and encrypted message
    std::vector<unsigned char> combined(iv.begin(), iv.end());
    combined.insert(combined.end(), encrypted, encrypted + decryptedLen);

    // 5. Encode the combined message using Base64
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);
    BIO_write(b64, combined.data(), static_cast<int>(combined.size()));
    BIO_flush(b64);

    BUF_MEM* buffer;
    BIO_get_mem_ptr(b64, &buffer);

    std::string encoded(buffer->data, buffer->length);

    // Clean up
    delete[] encrypted;
    BIO_free_all(b64);

    return encoded;
}

EVP_PKEY* SecureTransmitter::transformStringKey(const std::string& publicKeyStr) {
    // Create a new BIO memory buffer and load the public key string into it
    BIO* publicKeyBio = BIO_new_mem_buf(publicKeyStr.data(), static_cast<int>(publicKeyStr.size()));
    if (!publicKeyBio) {
        throw std::runtime_error("Failed to create BIO memory buffer");
    }

    // Read the public key from the BIO memory buffer
    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(publicKeyBio, nullptr, nullptr, nullptr);
    BIO_free(publicKeyBio);

    if (!publicKey) {
        throw std::runtime_error("Failed to read server's public key");
    }

    return publicKey;
}

std::string SecureTransmitter::decryptMessage(const std::string& encryptedMessage, std::vector<unsigned char> sharedKey) {
    // Decode the Base64-encoded encrypted message
    BIO* bio = BIO_new_mem_buf(encryptedMessage.data(), static_cast<int>(encryptedMessage.size()));
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, bio);

    std::vector<unsigned char> decodedMessage(encryptedMessage.size());
    int decodedLen = BIO_read(b64, decodedMessage.data(), static_cast<int>(decodedMessage.size()));
    BIO_free_all(b64);

    if (decodedMessage.size() < 16 || decodedLen <= 16) {
        return encryptedMessage;
    }
    // Separate the IV and encrypted message
    std::vector<unsigned char> iv(decodedMessage.begin(), decodedMessage.begin() + 16);
    std::vector<unsigned char> encryptedMessageBinary(decodedMessage.begin() + 16, decodedMessage.begin() + decodedLen);


    // Initialize the context for decryption
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug()<<"Failed to create cipher context for decryption";
        return encryptedMessage;
    }

    // Set the decryption key and IV
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, sharedKey.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        qDebug()<<"Failed to set decryption key and IV";
        return encryptedMessage;
    }

    // Decrypt the message
    std::vector<unsigned char> decryptedMessage(encryptedMessageBinary.size());
    int decryptedLen = 0;

    if (!EVP_DecryptUpdate(ctx, decryptedMessage.data(), &decryptedLen, encryptedMessageBinary.data(), static_cast<int>(encryptedMessageBinary.size()))) {
        EVP_CIPHER_CTX_free(ctx);
        qDebug()<<"Failed to decrypt message";
        return encryptedMessage;
    }

    int final_len = 0;
    decryptedMessage.resize(decryptedLen + EVP_MAX_BLOCK_LENGTH);
    if (!EVP_DecryptFinal_ex(ctx, decryptedMessage.data() + decryptedLen, &final_len)) {
        print_openssl_error();
        EVP_CIPHER_CTX_free(ctx);
        qDebug()<<"Failed to finalize decryption";
        return encryptedMessage;
    }

    decryptedMessage.resize(decryptedLen + final_len);
    EVP_CIPHER_CTX_free(ctx);
    return std::string(decryptedMessage.begin(), decryptedMessage.end());
}

void SecureTransmitter::setPrivateKey(const std::string& privateKeyFilePath)
{
    // Read the client's private key from the file
    BIO* privateKeyBio = BIO_new_file(privateKeyFilePath.c_str(), "r");
    if (!privateKeyBio) {
        throw std::runtime_error("Failed to open private key file");
    }
    privateKey_ = PEM_read_bio_PrivateKey(privateKeyBio, nullptr, nullptr, nullptr);
    BIO_free(privateKeyBio);

    if (!privateKey_) {
        throw std::runtime_error("Failed to read private key");
    }
}

void SecureTransmitter::print_openssl_error() {
    unsigned long err = ERR_get_error();
    if (err != 0) {
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        qDebug() << "OpenSSL error: " << err_buf ;
    } else {
        qDebug() << "No OpenSSL error available";
    }
}

std::string SecureTransmitter::generateKeys()
{
    // 1. Select the elliptic curve
    EC_KEY* ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!ec_key) {
        throw std::runtime_error("Error creating EC_KEY structure");
    }

    // 2. Generate the EC key pair
    if (EC_KEY_generate_key(ec_key) != 1) {
        EC_KEY_free(ec_key);
        throw std::runtime_error("Error generating EC key pair");
    }

    // 3. Extract the private and public keys
    EVP_PKEY* private_key = EVP_PKEY_new();
    EVP_PKEY_set1_EC_KEY(private_key, ec_key);

    // 4. Store the private key in the class member
    if (privateKey_) {
        EVP_PKEY_free(privateKey_);
    }
    privateKey_ = private_key;

    // 5. Export the public key to a string
    BIO* public_key_bio = BIO_new(BIO_s_mem());
    if (!public_key_bio) {
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to create BIO for public key export");
    }

    if (!PEM_write_bio_EC_PUBKEY(public_key_bio, ec_key)) {
        print_openssl_error();
        BIO_free(public_key_bio);
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to write public key to BIO");
    }

    BUF_MEM* public_key_mem;
    BIO_get_mem_ptr(public_key_bio, &public_key_mem);
    std::string public_key_str(public_key_mem->data, public_key_mem->length);

    // 6. Clean up
    BIO_free(public_key_bio);
    EC_KEY_free(ec_key);

    // 7. Return the public key as a string
    return public_key_str;
}

std::string SecureTransmitter::getPublicKeyFromPrivateKey() {
    if (!privateKey_) {
        throw std::runtime_error("Private key is not set");
    }

    // Get the EC_KEY from the EVP_PKEY structure
    EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(privateKey_);
    if (!ec_key) {
        throw std::runtime_error("Failed to extract EC_KEY from EVP_PKEY");
    }

    // Extract the public key component
    const EC_POINT* public_key_point = EC_KEY_get0_public_key(ec_key);
    if (!public_key_point) {
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to get public key from EC_KEY");
    }

    // Convert the public key point to a string
    BIO* public_key_bio = BIO_new(BIO_s_mem());
    if (!public_key_bio) {
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to create BIO for public key");
    }

    if (!PEM_write_bio_EC_PUBKEY(public_key_bio, ec_key)) {
        BIO_free(public_key_bio);
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to write public key to BIO");
    }

    BUF_MEM* public_key_mem;
    BIO_get_mem_ptr(public_key_bio, &public_key_mem);
    std::string public_key_str(public_key_mem->data, public_key_mem->length);

    // Clean up
    BIO_free(public_key_bio);
    EC_KEY_free(ec_key);

    return public_key_str;
}
