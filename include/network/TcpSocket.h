#ifndef NCP_TCPSOCKET_H
#define NCP_TCPSOCKET_H

#include <string>

class TcpSocket {
private:
    int _socketFd;
    int _remotePort;
    std::string _remoteIP;
public:
    TcpSocket();
    TcpSocket(int socketFd);
    TcpSocket(int socketFd, std::string remoteIP, int remotePort);
    ~TcpSocket();
    void connectTo(std::string &host, int port);
    void bindTo(int port);
    void listenForConnections(int backlog = 5);
    TcpSocket acceptConnection();

    void sendData(std::string& data);
    std::string receiveData(size_t bufferSize);

    void closeSocket();

    void printAddress(const std::string& connectionSide);

    int getSocketFd();
    bool isSocketValid();
    int releaseSocketFd();
};

#endif //NCP_TCPSOCKET_H