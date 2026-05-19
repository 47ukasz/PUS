#include <client/Client.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>
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

void Client::connect() {
    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = createClientContext();

    _socket.connectTo(_host, _port);

    cout << "[KLIENT] Uzyskano połączenie TCP z serwerem(" << _host << ":" << _port << ")" << endl;

    TlsConnection client_tls(ctx, _socket.getSocketFd());
    client_tls.connectTls();

    cout << "[KLIENT] Uzyskano szyfrowane połączenie TLS" << endl;

    Message msg {MessageType::PING, "1", time(nullptr), "1231", {"text", "PING"}};
    string message = JsonCoder::serialize(msg);

    client_tls.sendData(message);

    cout << "[KLIENT] Wysłano wiadomość" << endl;

    string response = client_tls.receiveData(4096);
    Message responseMsg = JsonCoder::deserialize(response);

    cout << "[KLIENT] Otrzymano wiadomość: " << responseMsg._payload.dump() << endl;

    SSL_CTX_free(ctx);
}
