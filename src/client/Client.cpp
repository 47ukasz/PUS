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

void Client::connect() {
    SSL_library_init();
    SSL_load_error_strings();

    _ctx = createClientContext();

    _socket.connectTo(_host, _port);

    _tls = make_unique<TlsConnection>(_ctx, _socket.getSocketFd());;
    _tls->connectTls();

    cout << "[KLIENT] Uzyskano szyfrowane połączenie TLS [" << _host << ":" << _port << "]" << endl;
}

void Client::run() {
    connect();
    cout << "Dostępne komendy: login, attach, detach, status, stats, reset, ping, exit" << endl;
    string commandLine;

    while (true) {
        cout << "ncp> ";

        if (!getline(cin, commandLine)) {
            break;
        }

        if (commandLine.empty()) {
            continue;
        }

        handleCommand(commandLine);
    }
}

void Client::handleCommand(string &command) {
    Message message = CommandMapper::mapToMessage(command);

    string rawMessage = JsonCoder::serialize(message);
    _tls->sendData(rawMessage);

    if (message._type == MessageType::BYE) {
        cout << "[KLIENT] Zamykanie klienta" << endl;
        exit(EXIT_SUCCESS); // tymczasowo bo potem w to miejsce trzeba bedzie normalnie wsadzic rozlaczenie z serwerem i wyslanie komunikatu
    }

    string rawResponse = _tls->receiveData(4096);
    Message response = JsonCoder::deserialize(rawResponse);

    cout << "[KLIENT] Odpowiedź serwera: " << response._payload.dump() << endl;
}
