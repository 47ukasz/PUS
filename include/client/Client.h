#ifndef NCP_CLIENT_H
#define NCP_CLIENT_H
#include <memory>
#include <openssl/types.h>

#include "network/TcpSocket.h"
#include "network/TlsConnection.h"

class Client {
    private:
        std::string _host;
        int _port;
        TcpSocket _socket;
        SSL_CTX* _ctx;
        std::unique_ptr<TlsConnection> _tls;

        void handleCommand(std::string& command);
        void connect();
    public:
        Client(std::string host, int port);
        ~Client();
        void run();

        static SSL_CTX* createClientContext();
};

#endif //NCP_CLIENT_H