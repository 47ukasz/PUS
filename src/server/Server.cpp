#include <server/Server.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>

#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <csignal>
#include <string>
#include <unistd.h>

#include "protocol/JsonCoder.h"
#include "server/RequestHandler.h"

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

    SSL_CTX *ctx = createServerContext();

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

    Session session{};

    while (session.active) {
        try {
            string rawRequest = client.receiveData(4096);

            if (rawRequest.empty()) {
                cout << "[SERWER] Klient zamknął połączenie" << endl;
                break;
            }

            cout << "[SERWER] Odebrano komunikat: " << rawRequest << endl;

            Message request = JsonCoder::deserialize(rawRequest);
            Message response = RequestHandler::handleRequest(request, session);

            string rawResponse = JsonCoder::serialize(response);

            cout << "[SERWER] Wysłano odpowiedź: " << rawResponse << endl;
            client.sendData(rawResponse);

            if (request._type == MessageType::BYE) {
                cout << "[SERWER] Sesja zakończona przez klienta" << endl;
                break;
            }

        } catch (const exception& e) {
            cout << "[SERWER] Błąd obsługi klienta: " << e.what() << endl;
            break;
        }
    }
}