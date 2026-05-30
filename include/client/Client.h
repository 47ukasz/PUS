#ifndef NCP_CLIENT_H
#define NCP_CLIENT_H

#include <memory>
#include <openssl/types.h>
#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <mutex>

#include "PendingRequest.h"
#include "network/TcpSocket.h"
#include "network/TlsConnection.h"
#include "protocol/Message.h"

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

        std::map<std::string, PendingRequest> _pendingRequests;

        std::thread _receiverThread;
        std::atomic<bool> _isReceiverRunning { false };
        std::atomic<bool> _exitAfterSessionClosed { false };
        std::mutex _sendMutex;
        std::mutex _printMutex;
        std::mutex _pendingMutex;

        std::thread _timeoutThread;
        bool _isTimeoutRunning { false };

        void startTimeoutManager();
        void stopTimeoutManager();
        void timeoutLoop();

        void handleRequestTimeout(std::map<std::string, PendingRequest>::iterator& it, time_t now, const std::string& timeoutType);
        void handleCommand(std::string& command);
        void handleResponse(std::string& rawResponse);

        void connectToServer(const std::string& host, int port);
        void sendMessage(models::Message& message);

        void startReceiver();
        void stopReceiver();
        void receiverLoop();

        void updatePendingRequest(models::Message& response);
    public:
        Client(std::string host, int port);
        ~Client();
        void run();

        static SSL_CTX* createClientContext();
};

#endif //NCP_CLIENT_H