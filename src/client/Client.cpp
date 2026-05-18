#include <client/Client.h>
#include <network/TlsConnection.h>

#include <iostream>
#include <stdexcept>

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

    string message = "PING od klienta";
    client_tls.sendData(message);

    cout << "[KLIENT] Wysłano wiadomość" << endl;

    string response = client_tls.receiveData(4096);

    cout << "[KLIENT] Otrzymano wiadomość: " << response << endl;

    SSL_CTX_free(ctx);
}
