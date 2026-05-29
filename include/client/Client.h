#ifndef NCP_CLIENT_H
#define NCP_CLIENT_H

#include <memory>
#include <openssl/types.h>
#include <string>

#include "network/TcpSocket.h"
#include "network/TlsConnection.h"
#include "protocol/Message.h"
#include "protocol/TimeoutConfig.h"

class Client {
    private:
        std::string _host;
        int _port;
        TcpSocket _socket;
        SSL_CTX* _ctx = nullptr;
        std::unique_ptr<TlsConnection> _tls;

        bool _connected = false;
        bool _authenticated = false;
        std::string _sessionToken;

        void handleCommand(std::string& command);
        void connectToServer(const std::string& host, int port);
        void sendAndPrintResponse(models::Message& message);

        TimeoutType expectedTimeoutFor(MessageType type);

    public:
        Client(std::string host, int port);
        ~Client();
        void run();

        static SSL_CTX* createClientContext();
};

#endif //NCP_CLIENT_H