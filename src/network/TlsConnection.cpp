#include <network/TlsConnection.h>

#include <iostream>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>

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

void TlsConnection::setReceiveTimeout(int timeoutSeconds) {
    timeval timeout{};
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;

    if (setsockopt(_socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        throw runtime_error(strerror(errno));
    }
}

void TlsConnection::clearReceiveTimeout() {
    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (setsockopt(_socketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
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
        int sslError = SSL_get_error(_ssl, returnValue);

        if (sslError == SSL_ERROR_WANT_READ || errno == EAGAIN || errno == EWOULDBLOCK) {
            throw runtime_error("TIMEOUT");
        }

        throw runtime_error(strerror(errno));

    }

    buffer.resize(returnValue);
    return buffer;
}
