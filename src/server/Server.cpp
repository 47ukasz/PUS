#include <server/Server.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>

#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include "protocol/JsonCoder.h"

using namespace std;
using namespace models;

SSL_CTX *Server::createServerContext() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());

    if (!ctx) {
        throw runtime_error("Nie udało się utworzyć kontekstu po stronie serwera.");
    }

    if (SSL_CTX_use_certificate_file(ctx, "certs/server.crt", SSL_FILETYPE_PEM) <= 0) {
        throw runtime_error("Nie udało się wczytać certyfikatu serwera.");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "certs/server.key", SSL_FILETYPE_PEM) <= 0) {
        throw runtime_error("Nie udało się wczytać klucza prywatnego.");
    }

    return ctx;
}

Server::Server(int port, int backlog) : _port(port), _backlog(backlog) {}

void Server::start() {
    signal(SIGCHLD, SIG_IGN);
    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = Server::createServerContext();

    _socket.bindTo(_port);
    _socket.listenForConnections(_backlog);

    cout << "[SERWER] Nasłuchuje na porcie " << _port << " ..." << endl;

    while (true) {
        TcpSocket client = _socket.acceptConnection();
        client.printAddress("KLIENT");

        pid_t pid = fork();

        if (pid < 0) {
            throw runtime_error("Nie udało się utworzyć procesu");
        }

        if (pid == 0) {
            _socket.closeSocket();

            TlsConnection client_tls(ctx, client.getSocketFd());
            client_tls.acceptTls();

            handleClient(client_tls);
            return;
        }

        client.closeSocket();
    }

    SSL_CTX_free(ctx);
}

void Server::handleClient(TlsConnection &client) {
    cout << "[SERWER] Klient uzyskał połączenie TLS" << endl;

    string message = client.receiveData(4096);
    Message msg = JsonCoder::deserialize(message);

    if (msg._type == MessageType::PING) {
        cout << "[SERWER] Odebrano PING" << endl;

        Message response {MessageType::PONG,msg._message_id,time(nullptr),msg._session_token,{{"text", "PONG"}}};

        string responseRaw = JsonCoder::serialize(response);

        cout << "[SERWER] Wysłano wiadomość" << endl;
        client.sendData(responseRaw);
    }
}
