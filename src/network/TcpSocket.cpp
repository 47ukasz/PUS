#include <network/TcpSocket.h>

#include <iostream>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

TcpSocket::TcpSocket() : _socketFd(socket(AF_INET, SOCK_STREAM, 0)) {
    if (_socketFd < 0) {
        throw runtime_error(strerror(errno));
    }
}

TcpSocket::TcpSocket(int socketFd) : _socketFd(socketFd) {}

TcpSocket::TcpSocket(int socketFd, string remoteIP, int remotePort) : _socketFd(socketFd), _remoteIP(remoteIP), _remotePort(remotePort) {}

TcpSocket::~TcpSocket() {
    closeSocket();
}

void TcpSocket::connectTo(string &host, int port) {
    sockaddr_in serverAddress{};
    int returnValue = 0;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    returnValue = inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr);

    if (returnValue <= 0) {
        throw runtime_error(strerror(errno));
    }

    returnValue = connect(_socketFd, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));

    if (returnValue < 0) {
        throw runtime_error(strerror(errno));
    }
}

void TcpSocket::bindTo(int port) {
    sockaddr_in serverAddress{};
    int returnValue = 0;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    returnValue = bind(_socketFd, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress));

    if (returnValue < 0) {
        throw runtime_error(strerror(errno));
    }
}

void TcpSocket::listenForConnections(int backlog) {
    int returnValue = 0;

    returnValue = listen(_socketFd, backlog);

    if (returnValue < 0) {
        throw runtime_error(strerror(errno));
    }
}

TcpSocket TcpSocket::acceptConnection() {
    sockaddr_in clientAddress{};
    socklen_t clientAddressLength = sizeof(clientAddress);

    int clientFd = accept(_socketFd, reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressLength);

    if (clientFd < 0) {
        throw runtime_error(strerror(errno));
    }

    char clientIp[INET_ADDRSTRLEN];
    int clientPort = ntohs(clientAddress.sin_port);

    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, INET_ADDRSTRLEN);

    return TcpSocket(clientFd, clientIp, clientPort);
}

void TcpSocket::sendData(string &data) {
    int returnValue = 0;

    returnValue = send(_socketFd, data.c_str(), data.size(), 0);

    if (returnValue < 0) {
        throw runtime_error(strerror(errno));
    }
}

string TcpSocket::receiveData(size_t bufferSize) {
    string buffer(bufferSize, '\0');
    int returnValue = 0;

    returnValue = recv(_socketFd, buffer.data(), bufferSize, 0);

    if (returnValue < 0) {
        throw runtime_error(strerror(errno));
    }

    if (returnValue == 0) {
        return "";
    }

    buffer.resize(returnValue);
    return buffer;
}

void TcpSocket::closeSocket() {
    if (_socketFd >= 0) {
        close(_socketFd);
        _socketFd = -1;
    }
}

void TcpSocket::printAddress(const string& connectionSide) {
    cout << "[ADRES IP " << connectionSide << "] " << _remoteIP << ":" << _remotePort << endl;
}

int TcpSocket::getSocketFd() {
    return _socketFd;
}

int TcpSocket::releaseSocketFd() {
    int fd = _socketFd;
    _socketFd = -1;
    return fd;
}

bool TcpSocket::isSocketValid() {
    return _socketFd >= 0;
}

void TcpSocket::recreateSocket() {
    closeSocket();

    _socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_socketFd < 0) {
        throw runtime_error(strerror(errno));
    }
}