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
#include <atomic>
#include <ctime>

struct Session {
    bool helloDone = false;
    bool authenticated = false;
    std::string sessionToken;
    bool active = true;
    int failedAuthAttempts = 0;

    time_t rateWindowStart = time(nullptr);
    int requestsInCurrentWindow = 0;

    std::map<std::string, std::vector<models::Message>> processedMessages;
};

struct KeepAliveSession {
    std::mutex mutex;
    std::atomic<bool> running{true};
    int failedPings = 0;
    bool waitingForPong = false;

    time_t lastActivity = time(nullptr);
    time_t pingSentAt = 0;
};

class Server {
    private:
        int _port;
        int _backlog;
        TcpSocket _socket;
        UeSimulation _simulation;
        std::mutex _simulationMutex;

        void handleClient(TlsConnection &client);
        void keepAliveLoop(TlsConnection& client, Session& session, std::mutex& sendMutex, KeepAliveSession& keepAlive);

    public:
        Server(int port, int backlog);
        void start();
        static SSL_CTX* createServerContext();
};

#endif //NCP_SERVER_H