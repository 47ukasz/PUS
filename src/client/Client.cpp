#include <client/Client.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>
#include <client/CommandMapper.h>
#include <protocol/JsonCoder.h>

#include <iostream>
#include <stdexcept>

using namespace models;
using namespace std;

SSL_CTX *Client::createClientContext() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (!ctx) {
        throw runtime_error("Nie udało się utworzyć kontekstu po stronie klienta.");
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

    return ctx;
}

Client::Client(string host, int port) : _host(host), _port(port) {}

Client::~Client() {
    if (_ctx) {
        SSL_CTX_free(_ctx);
        _ctx = nullptr;
    }
}

void Client::connectToServer(const string& host, int port) {
    if (_connected) {
        lock_guard lock(_printMutex);
        cout << "[KLIENT] Połączenie jest już aktywne." << endl;
        return;
    }

    SSL_library_init();
    SSL_load_error_strings();

    _host = host;
    _port = port;

    _ctx = createClientContext();

    string hostCopy = _host;
    _socket.connectTo(hostCopy, _port);

    _tls = make_unique<TlsConnection>(_ctx, _socket.getSocketFd());
    _tls->connectTls();

    _connected = true;

    {
        lock_guard lock(_printMutex);
        cout << "[KLIENT] Uzyskano szyfrowane połączenie TLS [" << _host << ":" << _port << "]" << endl;
    }

    startReceiver();
    startTimeoutManager();
}

void Client::run() {
    {
        lock_guard lock(_printMutex);
        cout << "Dostępne komendy: connect <host> <port>, login <user> <password>, ping, exit" << endl;
    }

    string commandLine;

    while (true) {
        {
            lock_guard lock(_printMutex);
            cout << "ncp> ";
        }

        if (!getline(cin, commandLine)) {
            break;
        }

        if (commandLine.empty()) {
            continue;
        }

        try {
            handleCommand(commandLine);
        } catch (const exception& e) {
            lock_guard lock(_printMutex);
            cout << "[KLIENT] Błąd: " << e.what() << endl;
        }
    }

    stopReceiver();
    stopTimeoutManager();
}

void Client::updatePendingRequest(models::Message &response) {
    string messageId = response._message_id;

    if (messageId.empty()) {
        return;
    }

    lock_guard lock(_pendingMutex);

    auto it = _pendingRequests.find(messageId);

    if (it == _pendingRequests.end()) {
        return;
    }

    if (response._type == MessageType::ACK) {
        it->second.ackReceived = true;
        it->second.lastSentAt = time(nullptr);

        if (it->second.message._type == MessageType::HELLO || it->second.message._type == MessageType::BYE) {
            _pendingRequests.erase(it);
        }

        return;
    }

    if (response._type == MessageType::AUTH_OK || response._type == MessageType::AUTH_FAIL || response._type == MessageType::PONG || response._type == MessageType::RESULT || response._type == MessageType::ERROR) {
        it->second.completed = true;

        _pendingRequests.erase(it);
    }
}

void Client::handleCommand(string &command) {
    ParsedCommand parsed = CommandMapper::parse(command);

    Message message = parsed.message;

    if (message._type == MessageType::HELLO) {
        connectToServer(parsed.host, parsed.port);
        sendMessage(message);
        return;
    }

    if (!_connected) {
        lock_guard lock(_printMutex);
        cout << "[KLIENT] Najpierw połącz się z serwerem: connect <host> <port>" << endl;
        return;
    }

    if (message._type == MessageType::BYE) {
        if (!_connected) {
            lock_guard lock(_printMutex);
            cout << "[KLIENT] Zamykanie klienta." << endl;
            exit(EXIT_SUCCESS);
        }

        if (_authenticated) {
            message._session_token = _sessionToken;
            sendMessage(message);
        }

        lock_guard lock(_printMutex);
        cout << "[KLIENT] Zamykanie klienta." << endl;
        exit(EXIT_SUCCESS);
    }

    if (_authenticated && message._type != MessageType::AUTH) {
        message._session_token = _sessionToken;
    }

    sendMessage(message);
}

void Client::sendMessage(models::Message &message) {
    string rawMessage = JsonCoder::serialize(message);

    {
        lock_guard lock(_sendMutex);
        _tls->sendData(rawMessage);
    }

    PendingRequest pendingRequest;

    pendingRequest.message = message;
    pendingRequest.rawMessage = rawMessage;
    pendingRequest.lastSentAt = time(nullptr);

    {
        lock_guard lock(_pendingMutex);
        _pendingRequests[message._message_id] = pendingRequest;
    }
}

void Client::startReceiver() {
    if (_isReceiverRunning) {
        return;
    }

    _isReceiverRunning = true;
    _receiverThread = thread(&Client::receiverLoop, this);
}

void Client::stopReceiver() {
    _isReceiverRunning = false;

    if (_receiverThread.joinable()) {
        _receiverThread.join();
    }
}

void Client::handleResponse(std::string& rawResponse) {
    {
        lock_guard lock(_printMutex);
        cout << "[KLIENT] Odpowiedź serwera: " << rawResponse << endl;
    }

    Message response = JsonCoder::deserialize(rawResponse);
    updatePendingRequest(response);

    if (response._type == MessageType::PING) {
        Message pong{
            MessageType::PONG,
            response._message_id,
            time(nullptr),
            _sessionToken,
            {
                {"message", "PONG od klienta"}
            }
        };

        string rawPong = JsonCoder::serialize(pong);

        {
            lock_guard lock(_sendMutex);
            _tls->sendData(rawPong);
        }

        lock_guard lock(_printMutex);

        cout << "[KLIENT] Odebrano PING od serwera, wysłano PONG." << endl;
        return;
    }

    if (response._type == MessageType::AUTH_OK) {
        if (response._payload.contains("session_token")) {
            _sessionToken = response._payload.at("session_token");
            _authenticated = true;

            lock_guard lock(_printMutex);
            cout << "[KLIENT] Zalogowano poprawnie. Token sesji zapisany." << endl;
        }
    }

    if (response._type == MessageType::AUTH_FAIL) {
        _authenticated = false;
        _sessionToken.clear();

        lock_guard lock(_printMutex);
        cout << "[KLIENT] Logowanie nieudane." << endl;
    }

    if (response._type == MessageType::ERROR) {
        if (response._payload.contains("error_code") && response._payload.contains("error_message")) {

            lock_guard lock(_printMutex);
            cout << "[KLIENT] Błąd " << response._payload.at("error_code") << ": " << response._payload.at("error_message") << endl;
        }
    }

    lock_guard lock(_printMutex);
    cout << "ncp> " << flush;
}

void Client::receiverLoop() {
    while (_isReceiverRunning) {
        try {
            string rawResponse = _tls->receiveData(4096);

            handleResponse(rawResponse);
        } catch (const std::exception& e) {
            if (_isReceiverRunning) {
                lock_guard lock(_printMutex);
                cout << "[ERROR] Błąd odbierania: " << e.what() << endl;
            }

            _isReceiverRunning = false;

            break;
        }
    }
}

void Client::startTimeoutManager() {
    if (_isTimeoutRunning) {
        return;
    }

    _isTimeoutRunning = true;
    _timeoutThread = thread(&Client::timeoutLoop, this);
}

void Client::stopTimeoutManager() {
    _isTimeoutRunning = false;

    if (_timeoutThread.joinable()) {
        _timeoutThread.join();
    }
}

void Client::timeoutLoop() {
    while (_isTimeoutRunning) {
        time_t now = time(nullptr);

        {
            lock_guard lock(_pendingMutex);

            for (auto it = _pendingRequests.begin(); it != _pendingRequests.end(); ) {
                PendingRequest& pending = it->second;

                if (!pending.ackReceived && now - pending.lastSentAt >= 2) {

                    lock_guard printLock(_printMutex);
                    cout << "[KLIENT] Timeout ACK dla message_id=" << it->first << endl;

                    handleRequestTimeout(it, now);
                    continue;
                }

                if (pending.ackReceived && !pending.completed && now - pending.lastSentAt >= 10) {

                    lock_guard printLock(_printMutex);
                    cout << "[KLIENT] Timeout RESULT dla message_id=" << it->first << endl;

                    handleRequestTimeout(it, now);

                    continue;
                }

                ++it;
            }
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void Client::handleRequestTimeout(std::map<std::string, PendingRequest>::iterator &it, time_t now) {
    PendingRequest& pending = it->second;

    if (pending.retryCount >= 3) {
        lock_guard printLock(_printMutex);
        cout << "[KLIENT] Przekroczono limit retransmisji dla message_id="<< it->first << endl;

        it = _pendingRequests.erase(it);
        return;
    }

    {
        lock_guard sendLock(_sendMutex);
        _tls->sendData(pending.rawMessage);
    }

    pending.retryCount++;
    pending.lastSentAt = now;

    lock_guard printLock(_printMutex);

    cout << "[KLIENT] Retransmisja " << pending.retryCount << "/3 dla message_id=" << it->first << endl;

    ++it;
}

