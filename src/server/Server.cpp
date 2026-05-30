#include <server/Server.h>
#include <network/TlsConnection.h>
#include <protocol/Message.h>
#include <server/Logger.h>

#include <openssl/ssl.h>

#include <iostream>
#include <stdexcept>
#include <csignal>
#include <string>
#include <unistd.h>
#include <vector>
#include <thread>
#include <nlohmann/json.hpp>

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
    Logger::init("logs/server.log");
    Logger::info("Uruchamianie serwera NCP.");

    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = createServerContext();

    _socket.bindTo(_port);
    _socket.listenForConnections(_backlog);

    Logger::info("Serwer nasłuchuje na porcie " + to_string(_port) + ".");

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
    Logger::info("Klient uzyskał połączenie TLS.");

    Session session{};
    KeepAliveSession keepAlive;
    mutex sendMutex;

    thread keepAliveThread(&Server::keepAliveLoop, this, ref(client), ref(session), ref(sendMutex), ref(keepAlive));

    while (session.active) {
        try {
            string rawRequest = client.receiveData(4096);

            if (rawRequest.empty()) {
                cout << "[SERWER] Klient zamknął połączenie" << endl;
                break;
            }

            string logRequest = rawRequest;

            try {
                nlohmann::json requestJson = nlohmann::json::parse(rawRequest);

                if (requestJson.contains("type") && requestJson["type"] == "AUTH") {
                    if (requestJson.contains("payload") &&
                        requestJson["payload"].contains("password")) {
                        requestJson["payload"]["password"] = "***";
                    }
                }

                if (requestJson.contains("session_token")) {
                    requestJson["session_token"] = "***";
                }

                logRequest = requestJson.dump();

            } catch (...) {
                logRequest = rawRequest;
            }

            Logger::info("Odebrano komunikat: " + logRequest);

            Message request = JsonCoder::deserialize(rawRequest);

            {
                lock_guard lock(keepAlive.mutex);
                keepAlive.lastActivity = time(nullptr);

                if (request._type == MessageType::PONG) {
                    keepAlive.waitingForPong = false;
                    keepAlive.failedPings = 0;

                    Logger::info("Odebrano PONG od klienta.");
                    continue;
                }
            }

            vector<Message> responses = RequestHandler::handleRequest(request, session, _simulation, _simulationMutex);

            for (Message& response : responses) {
                string rawResponse = JsonCoder::serialize(response);

                Logger::info("Wysłano odpowiedź: " + rawResponse);

                lock_guard sendLock(sendMutex);

                client.sendData(rawResponse);
            }

            if (request._type == MessageType::BYE) {
                Logger::info("Sesja zakończona przez klienta.");

                break;
            }

        } catch (const exception& e) {
            Logger::error(string("Błąd obsługi klienta: ") + e.what());
            break;
        }
    }

    keepAlive.running = false;

    if (keepAliveThread.joinable()) {
        keepAliveThread.join();
    }
}

void Server::keepAliveLoop(TlsConnection& client, Session& session, mutex& sendMutex, KeepAliveSession& keepAlive) {
    int idleTimeoutSeconds = 15;
    int pongTimeoutSeconds = 5;
    int maxFailedPings = 3;

    while (keepAlive.running && session.active) {
        time_t now = time(nullptr);

        {
            lock_guard lock(keepAlive.mutex);

            if (!session.authenticated) {
                keepAlive.lastActivity = now;
                continue;
            }

            if (!keepAlive.waitingForPong && now - keepAlive.lastActivity >= idleTimeoutSeconds) {

                Message ping{
                    MessageType::PING,
                    "server_ping_" + to_string(now),
                    now,
                    session.sessionToken,
                    {
                        {"message", "PING od serwera"}
                    }
                };

                string rawPing = JsonCoder::serialize(ping);

                try {
                    {
                        lock_guard sendLock(sendMutex);
                        client.sendData(rawPing);
                    }

                    Logger::info("Brak aktywności. Wysłano PING: " + rawPing);

                    keepAlive.waitingForPong = true;
                    keepAlive.pingSentAt = now;
                } catch (const exception& e) {
                    cout << "[SERWER] Błąd wysyłania PING: " << e.what() << endl;

                    session.active = false;
                    keepAlive.running = false;
                    break;
                }
            }

            if (keepAlive.waitingForPong && now - keepAlive.pingSentAt >= pongTimeoutSeconds) {
                keepAlive.failedPings++;

                Logger::warn("Brak PONG. Nieudana próba: " + to_string(keepAlive.failedPings) + "/" + to_string(maxFailedPings));

                keepAlive.waitingForPong = false;
                keepAlive.lastActivity = now;

                if (keepAlive.failedPings >= maxFailedPings) {
                    Logger::warn("Przekroczono limit keep-alive. Zamykam sesję.");

                    session.active = false;
                    keepAlive.running = false;
                    break;
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(200));
    }
}
