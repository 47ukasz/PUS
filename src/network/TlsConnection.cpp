#include <network/TlsConnection.h>

#include <iostream>
#include <cerrno>
#include <cstring>
#include <stdexcept>

using namespace std;

TlsConnection::TlsConnection(SSL_CTX *ctx, int socketFd) : _ctx(ctx), _socketFd(socketFd), _ssl(nullptr){
    _ssl = SSL_new(ctx);

    if (!_ssl) {
        throw runtime_error(strerror(errno));
    }

    SSL_set_fd(_ssl, socketFd);
}

TlsConnection::~TlsConnection() {
    if (_ssl) {
        SSL_shutdown(_ssl);
        SSL_free(_ssl);
        _ssl = nullptr;
    }
}

void TlsConnection::acceptTls() {
    int returnValue = 0;

    returnValue = SSL_accept(_ssl);

    if (returnValue <= 0) {
        throw runtime_error(strerror(errno));
    }
}

void TlsConnection::connectTls() {
    int returnValue = 0;

    returnValue = SSL_connect(_ssl);

    if (returnValue <= 0) {
        throw runtime_error(strerror(errno));
    }
}

void TlsConnection::sendData(string &data) {
    int returnValue = 0;

    returnValue = SSL_write(_ssl, data.c_str(), data.size());

    if (returnValue <= 0) {
        throw runtime_error(strerror(errno));
    }
}

string TlsConnection::receiveData(size_t bufferSize) {
    string buffer(bufferSize, '\0');
    int returnValue = 0;

    returnValue = SSL_read(_ssl, buffer.data(), bufferSize);

    if (returnValue <= 0) {
        throw runtime_error(strerror(errno));
    }

    buffer.resize(returnValue);
    return buffer;
}
