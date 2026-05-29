#ifndef NCP_TLSCONNECTION_H
#define NCP_TLSCONNECTION_H

#include <openssl/ssl.h>
#include <string>

class TlsConnection {
    private:
        SSL_CTX* _ctx;
        SSL* _ssl;
        int _socketFd;
    public:
    TlsConnection(SSL_CTX* ctx, int socketFd);
    ~TlsConnection();

    void acceptTls();
    void connectTls();

    void setReceiveTimeout(int timeoutSeconds);
    void clearReceiveTimeout();

    void sendData(std::string& data);
    std::string receiveData(size_t bufferSize);
};

#endif //NCP_TLSCONNECTION_H