#ifndef NCP_SERVER_H
#define NCP_SERVER_H

#include <network/TcpSocket.h>
#include <network/TlsConnection.h>

class Server {
    private:
        int _port;
        int _backlog;
        TcpSocket _socket;

        void handleClient(TlsConnection &client);
    public:
        Server(int port, int backlog);
        void start();
        static SSL_CTX* createServerContext();
};

#endif //NCP_SERVER_H