#include <client/Client.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>
#include <client/CommandMapper.h>
#include <protocol/JsonCoder.h>

#include <iostream>
#include <stdexcept>
#include <sstream>

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

    cout << "[KLIENT] Uzyskano szyfrowane połączenie TLS [" << _host << ":" << _port << "]" << endl;
}

void Client::run() {
    cout << "Dostępne komendy: connect <host> <port>, login <user> <password>, ping, exit" << endl;

    string commandLine;

    while (true) {
        cout << "ncp> ";

        if (!getline(cin, commandLine)) {
            break;
        }

        if (commandLine.empty()) {
            continue;
        }

        try {
            handleCommand(commandLine);
        } catch (const exception& e) {
            cout << "[KLIENT] Błąd: " << e.what() << endl;
        }
    }
}

void Client::handleCommand(string &command) {
    if (command.rfind("connect ", 0) == 0) {
        istringstream iss(command);
        string cmd;
        string host;
        int port;

        iss >> cmd >> host >> port;

        if (host.empty() || port <= 0) {
            throw runtime_error("Użycie: connect <host> <port>");
        }

        connectToServer(host, port);

        Message hello = CommandMapper::mapToMessage(command);
        sendAndPrintResponse(hello);
        return;
    }

    if (command == "exit") {
        if (!_connected) {
            cout << "[KLIENT] Zamykanie klienta." << endl;
            exit(EXIT_SUCCESS);
        }

        if (!_authenticated) {
            cout << "[KLIENT] Połączenie nie było zalogowane. Zamykanie klienta lokalnie." << endl;
            exit(EXIT_SUCCESS);
        }

        Message message = CommandMapper::mapToMessage(command);
        message._session_token = _sessionToken;

        sendAndPrintResponse(message);

        cout << "[KLIENT] Zamykanie klienta." << endl;
        exit(EXIT_SUCCESS);
    }

    if (!_connected) {
        cout << "[KLIENT] Najpierw połącz się z serwerem: connect <host> <port>" << endl;
        return;
    }

    Message message = CommandMapper::mapToMessage(command);

    if (_authenticated && message._type != MessageType::AUTH) {
        message._session_token = _sessionToken;
    }

    sendAndPrintResponse(message);
}

void Client::sendAndPrintResponse(Message& message) {
    string rawMessage = JsonCoder::serialize(message);
    _tls->sendData(rawMessage);

    string rawResponse = _tls->receiveData(4096);
    Message response = JsonCoder::deserialize(rawResponse);

    cout << "[KLIENT] Odpowiedź serwera: " << rawResponse << endl;

    if (response._type == MessageType::AUTH_OK) {
        if (response._payload.contains("session_token")) {
            _sessionToken = response._payload.at("session_token");
            _authenticated = true;

            cout << "[KLIENT] Zalogowano poprawnie. Token sesji zapisany." << endl;
        }
    }

    if (response._type == MessageType::AUTH_FAIL) {
        _authenticated = false;
        _sessionToken.clear();

        cout << "[KLIENT] Logowanie nieudane." << endl;
    }

    bool operationWithAckAndResult =
        message._type == MessageType::ATTACH ||
        message._type == MessageType::DETACH ||
        message._type == MessageType::RESET_SIM;

    if (operationWithAckAndResult && response._type == MessageType::ACK) {
        if (response._payload.contains("status") && response._payload.at("status") == "PROCESSING") {
            string rawFinalResponse = _tls->receiveData(4096);
            Message finalResponse = JsonCoder::deserialize(rawFinalResponse);

            cout << "[KLIENT] Końcowa odpowiedź serwera: " << rawFinalResponse << endl;
        }
    }

    if (response._type == MessageType::ERROR) {
        if (response._payload.contains("error_code") && response._payload.contains("error_message")) {
            cout << "[KLIENT] Błąd "
                 << response._payload.at("error_code")
                 << ": "
                 << response._payload.at("error_message")
                 << endl;
        }
    }
}