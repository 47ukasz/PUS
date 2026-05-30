#include <server/RequestHandler.h>
#include <server/MessageValidator.h>
#include <server/ValidationException.h>
#include <server/Logger.h>

#include <ctime>
#include <string>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <iomanip>
#include <random>
#include <sstream>

using namespace models;
using namespace std;

vector<Message> RequestHandler::handleRequest(Message& request, Session& session, UeSimulation& simulation, mutex& simulationMutex) {
    try {
        MessageValidator::validate(request);

        if (hasProcessedMessage(session, request)) {
            Logger::warn("Wykryto duplikat message_id=" + request._message_id + ". Zwracam poprzednią odpowiedź.");
            return session.processedMessages[request._message_id];
        }

        switch (request._type) {
            case MessageType::HELLO: {
                if (session.helloDone) {
                    vector<Message> responses = {
                        makeError(request, "INVALID_STATE", "HELLO was already exchanged.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                session.helloDone = true;

                vector<Message> responses = {
                    makeResponse(request, MessageType::ACK, {
                        {"status", "HELLO_ACCEPTED"}
                    })
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }

            case MessageType::AUTH: {
                if (!session.helloDone) {
                    vector<Message> responses = {
                        makeError(request, "INVALID_STATE", "HELLO must be sent before AUTH.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                string login = request._payload.at("login");
                string password = request._payload.at("password");

                if (login == "admin" && password == "admin") {
                    session.authenticated = true;
                    session.sessionToken = generateSessionToken();

                    Logger::info("AUTH_OK dla użytkownika: " + login);

                    vector<Message> responses = {
                        makeResponse(request, MessageType::AUTH_OK, {
                            {"session_token", session.sessionToken}
                        })
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                Logger::warn("AUTH_FAIL dla użytkownika: " + login);

                vector<Message> responses = {
                    makeResponse(request, MessageType::AUTH_FAIL, {
                        {"message", "Invalid login or password"}
                    })
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }

            case MessageType::BYE: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                session.active = false;

                Logger::info("BYE odebrano. Sesja zostanie zakończona.");

                vector<Message> responses = {
                    makeResponse(request, MessageType::ACK, {
                        {"status", "SESSION_CLOSED"}
                    })
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }

            case MessageType::PING: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                vector<Message> responses = {
                    makeResponse(request, MessageType::PONG, {
                        {"message", "PONG od serwera"}
                    })
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }

            case MessageType::ATTACH: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    string ueId = request._payload.at("ue_id");

                    lock_guard<mutex> lock(simulationMutex);
                    nlohmann::json result = simulation.attach(ueId);
                    Logger::info("ATTACH wykonano dla " + ueId + ".");

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    vector<Message> responses = {ack, finalResult};

                    storeProcessedMessage(session, request, responses);
                    return responses;

                } catch (const runtime_error& e) {
                    string error = e.what();

                    vector<Message> responses;

                    if (error.rfind("NOT_FOUND:", 0) == 0) {
                        responses = {
                            ack,
                            makeError(request, "NOT_FOUND", error.substr(11))
                        };
                    } else if (error.rfind("INVALID_STATE:", 0) == 0) {
                        responses = {
                            ack,
                            makeError(request, "INVALID_STATE", error.substr(15))
                        };
                    } else {
                        responses = {
                            ack,
                            makeError(request, "INTERNAL_ERROR", error)
                        };
                    }

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }
            }

            case MessageType::DETACH: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    string ueId = request._payload.at("ue_id");

                    lock_guard<mutex> lock(simulationMutex);
                    nlohmann::json result = simulation.detach(ueId);
                    Logger::info("DETACH wykonano dla " + ueId + ".");

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    vector<Message> responses = {ack, finalResult};
                    storeProcessedMessage(session, request, responses);
                    return responses;

                } catch (const runtime_error& e) {
                    string error = e.what();

                    vector<Message> responses;

                    if (error.rfind("NOT_FOUND:", 0) == 0) {
                        responses = {
                            ack,
                            makeError(request, "NOT_FOUND", error.substr(11))
                        };
                    } else if (error.rfind("INVALID_STATE:", 0) == 0) {
                        responses = {
                            ack,
                            makeError(request, "INVALID_STATE", error.substr(15))
                        };
                    } else {
                        responses = {
                            ack,
                            makeError(request, "INTERNAL_ERROR", error)
                        };
                    }

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }
            }

            case MessageType::STATUS: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                try {
                    string ueId = request._payload.at("ue_id");

                    lock_guard<mutex> lock(simulationMutex);
                    nlohmann::json result = simulation.status(ueId);
                    Logger::info("STATUS pobrano dla " + ueId + ".");

                    vector<Message> responses = {
                        makeResponse(request, MessageType::RESULT, result)
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;

                } catch (const runtime_error& e) {
                    string error = e.what();

                    vector<Message> responses;

                    if (error.rfind("NOT_FOUND:", 0) == 0) {
                        responses = {
                            makeError(request, "NOT_FOUND", error.substr(11))
                        };
                    } else if (error.rfind("INVALID_STATE:", 0) == 0) {
                        responses = {
                            makeError(request, "INVALID_STATE", error.substr(15))
                        };
                    } else {
                        responses = {
                            makeError(request, "INTERNAL_ERROR", error)
                        };
                    }

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }
            }

            case MessageType::GET_STATS: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                lock_guard<mutex> lock(simulationMutex);
                nlohmann::json result = simulation.stats();
                Logger::info("GET_STATS wykonano.");

                vector<Message> responses = {
                    makeResponse(request, MessageType::RESULT, result)
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }

            case MessageType::RESET_SIM: {
                if (!isAuthorized(request, session)) {
                    vector<Message> responses = {
                        makeError(request, "UNAUTHORIZED", "Operation requires valid authentication token.")
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    lock_guard<mutex> lock(simulationMutex);
                    nlohmann::json result = simulation.reset();
                    Logger::info("RESET_SIM wykonano.");

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    vector<Message> responses = {ack, finalResult};
                    storeProcessedMessage(session, request, responses);
                    return responses;

                } catch (const runtime_error& e) {
                    vector<Message> responses = {
                        ack,
                        makeError(request, "INTERNAL_ERROR", e.what())
                    };

                    storeProcessedMessage(session, request, responses);
                    return responses;
                }
            }

            default: {
                vector<Message> responses = {
                    makeError(request, "INVALID_STATE", "Unsupported message type.")
                };

                storeProcessedMessage(session, request, responses);
                return responses;
            }
        }

    } catch (const ValidationException& e) {
        vector<Message> responses = {
            makeError(request, e.errorCode(), e.what())
        };

        storeProcessedMessage(session, request, responses);
        return responses;

    } catch (const runtime_error& e) {
        string error = e.what();
        vector<Message> responses;

        if (error.rfind("NOT_FOUND:", 0) == 0) {
            responses = {
                makeError(request, "NOT_FOUND", error.substr(11))
            };
        } else if (error.rfind("INVALID_STATE:", 0) == 0) {
            responses = {
                makeError(request, "INVALID_STATE", error.substr(15))
            };
        } else {
            responses = {
                makeError(request, "INVALID_FORMAT", error)
            };
        }

        storeProcessedMessage(session, request, responses);
        return responses;

    } catch (const exception& e) {
        vector<Message> responses = {
            makeError(request, "INTERNAL_ERROR", e.what())
        };

        storeProcessedMessage(session, request, responses);
        return responses;
    }
}

Message RequestHandler::makeResponse(Message& request, MessageType type, nlohmann::json payload) {
    return Message{
        type,
        request._message_id,
        time(nullptr),
        request._session_token,
        payload
    };
}

Message RequestHandler::makeError(Message& request, string errorCode, string errorMessage) {
    return Message{
        MessageType::ERROR,
        request._message_id,
        time(nullptr),
        request._session_token,
        {
            {"error_code", errorCode},
            {"error_message", errorMessage}
        }
    };
}

string RequestHandler::generateSessionToken() {
    static random_device rd;
    static mt19937_64 generator(rd());
    static mutex tokenMutex;

    lock_guard<mutex> lock(tokenMutex);

    uniform_int_distribution<unsigned long long> distribution;

    stringstream ss;
    ss << "token_"
       << hex
       << setw(16) << setfill('0') << distribution(generator)
       << setw(16) << setfill('0') << distribution(generator);

    return ss.str();
}

bool RequestHandler::isAuthorized(Message& request, Session& session) {
    return session.authenticated && request._session_token == session.sessionToken;
}

bool RequestHandler::hasProcessedMessage(Session& session, Message& request) {
    return session.processedMessages.find(request._message_id) != session.processedMessages.end();
}

void RequestHandler::storeProcessedMessage(Session& session, Message& request, const vector<Message>& responses) {
    if (!request._message_id.empty()) {
        session.processedMessages[request._message_id] = responses;
    }
}