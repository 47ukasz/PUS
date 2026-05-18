#ifndef NCP_CLIENT_H
#define NCP_CLIENT_H
#include <openssl/types.h>

#include "network/TcpSocket.h"

class Client {
    private:
        std::string _host;
        int _port;
        TcpSocket _socket;
    public:
        Client(std::string host, int port);
        void connect();
        static SSL_CTX* createClientContext();
};

#endif //NCP_CLIENT_H