#ifndef NCP_SERVER_H
#define NCP_SERVER_H

#include <network/TcpSocket.h>
#include <network/TlsConnection.h>
#include <server/UeSimulation.h>
#include <protocol/Message.h>

#include <string>
#include <mutex>
#include <map>
#include <vector>

struct Session {
    bool helloDone = false;
    bool authenticated = false;
    std::string sessionToken;
    bool active = true;
    std::map<std::string, std::vector<models::Message>> processedMessages;
};

class Server {
    private:
        int _port;
        int _backlog;
        TcpSocket _socket;
        UeSimulation _simulation;
        std::mutex _simulationMutex;

        void handleClient(TlsConnection &client);

    public:
        Server(int port, int backlog);
        void start();
        static SSL_CTX* createServerContext();
};

#endif //NCP_SERVER_H