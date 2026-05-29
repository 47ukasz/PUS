#include <server/RequestHandler.h>
#include <server/MessageValidator.h>

#include <ctime>
#include <string>
#include <stdexcept>
#include <vector>

using namespace models;
using namespace std;

vector<Message> RequestHandler::handleRequest(Message& request, Session& session) {
    try {
        MessageValidator::validate(request);

        switch (request._type) {
            case MessageType::HELLO: {
                session.helloDone = true;

                return {
                    makeResponse(request, MessageType::ACK, {
                        {"status", "HELLO_ACCEPTED"}
                    })
                };
            }

            case MessageType::AUTH: {
                if (!session.helloDone) {
                    return {
                        makeError(request, "INVALID_STATE", "HELLO must be sent before AUTH.")
                    };
                }

                string login = request._payload.at("login");
                string password = request._payload.at("password");

                if (login == "admin" && password == "admin") {
                    session.authenticated = true;
                    session.sessionToken = generateSessionToken();

                    return {
                        makeResponse(request, MessageType::AUTH_OK, {
                            {"session_token", session.sessionToken}
                        })
                    };
                }

                return {
                    makeResponse(request, MessageType::AUTH_FAIL, {
                        {"message", "Invalid login or password"}
                    })
                };
            }

            case MessageType::BYE: {
                session.active = false;

                return {
                    makeResponse(request, MessageType::ACK, {
                        {"status", "SESSION_CLOSED"}
                    })
                };
            }

            case MessageType::PING: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                return {
                    makeResponse(request, MessageType::PONG, {
                        {"message", "PONG od serwera"}
                    })
                };
            }

            case MessageType::ATTACH: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    string ueId = request._payload.at("ue_id");
                    nlohmann::json result = session.simulation.attach(ueId);

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    return {ack, finalResult};

                } catch (const runtime_error& e) {
                    string error = e.what();

                    if (error.rfind("NOT_FOUND:", 0) == 0) {
                        return {
                            ack,
                            makeError(request, "NOT_FOUND", error.substr(11))
                        };
                    }

                    if (error.rfind("INVALID_STATE:", 0) == 0) {
                        return {
                            ack,
                            makeError(request, "INVALID_STATE", error.substr(15))
                        };
                    }

                    return {
                        ack,
                        makeError(request, "INTERNAL_ERROR", error)
                    };
                }
            }

            case MessageType::DETACH: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    string ueId = request._payload.at("ue_id");
                    nlohmann::json result = session.simulation.detach(ueId);

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    return {ack, finalResult};

                } catch (const runtime_error& e) {
                    string error = e.what();

                    if (error.rfind("NOT_FOUND:", 0) == 0) {
                        return {
                            ack,
                            makeError(request, "NOT_FOUND", error.substr(11))
                        };
                    }

                    if (error.rfind("INVALID_STATE:", 0) == 0) {
                        return {
                            ack,
                            makeError(request, "INVALID_STATE", error.substr(15))
                        };
                    }

                    return {
                        ack,
                        makeError(request, "INTERNAL_ERROR", error)
                    };
                }
            }

            case MessageType::STATUS: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                string ueId = request._payload.at("ue_id");
                nlohmann::json result = session.simulation.status(ueId);

                return {
                    makeResponse(request, MessageType::RESULT, result)
                };
            }

            case MessageType::GET_STATS: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                nlohmann::json result = session.simulation.stats();

                return {
                    makeResponse(request, MessageType::RESULT, result)
                };
            }

            case MessageType::RESET_SIM: {
                if (!session.authenticated) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Operation requires authentication.")
                    };
                }

                if (request._session_token != session.sessionToken) {
                    return {
                        makeError(request, "UNAUTHORIZED", "Invalid session token.")
                    };
                }

                Message ack = makeResponse(request, MessageType::ACK, {
                    {"status", "PROCESSING"}
                });

                try {
                    nlohmann::json result = session.simulation.reset();

                    Message finalResult = makeResponse(request, MessageType::RESULT, result);

                    return {ack, finalResult};

                } catch (const runtime_error& e) {
                    return {
                        ack,
                        makeError(request, "INTERNAL_ERROR", e.what())
                    };
                }
            }

            default:
                return {
                    makeError(request, "INVALID_STATE", "Unsupported message type.")
                };
        }

    } catch (const runtime_error& e) {
        string error = e.what();

        if (error.rfind("NOT_FOUND:", 0) == 0) {
            return {
                makeError(request, "NOT_FOUND", error.substr(11))
            };
        }

        if (error.rfind("INVALID_STATE:", 0) == 0) {
            return {
                makeError(request, "INVALID_STATE", error.substr(15))
            };
        }

        return {
            makeError(request, "INVALID_FORMAT", error)
        };

    } catch (const exception& e) {
        return {
            makeError(request, "INTERNAL_ERROR", e.what())
        };
    }
}

Message RequestHandler::makeResponse(Message& request, MessageType type, nlohmann::json payload) {
    return Message{type, request._message_id, time(nullptr), request._session_token, payload};
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
    return "token_" + to_string(time(nullptr));
}