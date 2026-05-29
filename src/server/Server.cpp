#include <server/Server.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>

#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <csignal>
#include <string>
#include <unistd.h>
#include <vector>
#include <thread>

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

Server::Server(int port, int backlog) : _port(port), _backlog(backlog), _simulation() {}

void Server::start() {
    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = createServerContext();

    _socket.bindTo(_port);
    _socket.listenForConnections(_backlog);

    cout << "[SERWER] Nasłuchuje na porcie " << _port << " ..." << endl;

    while (true) {
        TcpSocket client = _socket.acceptConnection();
        client.printAddress("KLIENT");

        int clientFd = client.releaseSocketFd();

        thread clientThread([this, ctx, clientFd]() {
            try {
                TlsConnection clientTls(ctx, clientFd);
                clientTls.acceptTls();

                handleClient(clientTls);

            } catch (const exception& e) {
                cout << "[SERWER] Błąd obsługi klienta: " << e.what() << endl;
            }

            close(clientFd);
        });

        clientThread.detach();
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
            vector<Message> responses = RequestHandler::handleRequest(request, session, _simulation, _simulationMutex);
            
            for (Message& response : responses) {
                string rawResponse = JsonCoder::serialize(response);
                cout << "[SERWER] Wysłano odpowiedź: " << rawResponse << endl;
                client.sendData(rawResponse);
            }

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