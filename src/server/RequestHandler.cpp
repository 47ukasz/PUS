#include <server/RequestHandler.h>
#include <server/MessageValidator.h>

#include <ctime>
#include <string>

using namespace models;
using namespace std;

Message RequestHandler::handleRequest(Message& request) {
    try {
        MessageValidator::validate(request);

        switch (request._type) {
            case MessageType::PING:
                return makeResponse(request, MessageType::PONG,{{"message", "PONG od serwera"}});

            case MessageType::AUTH:
                return makeResponse(request, MessageType::AUTH_OK,{{"session_token", "temporary-token"}});

            case MessageType::ATTACH:
                return makeResponse(request, MessageType::ACK,{
                        {"operation", "ATTACH"},
                        {"ue_id", request._payload.at("ue_id")},
                        {"status", "attached"}
                    }
                );

            case MessageType::DETACH:
                return makeResponse(request, MessageType::ACK,{
                        {"operation", "DETACH"},
                        {"ue_id", request._payload.at("ue_id")},
                        {"status", "detached"}
                    }
                );

            case MessageType::STATUS:
                return makeResponse(request, MessageType::RESULT,{
                        {"ue_id", request._payload.at("ue_id")},
                        {"state", "CONNECTED"}
                    }
                );

            case MessageType::GET_STATS:
                return makeResponse(request, MessageType::RESULT,{
                        {"connected_ues", 1},
                        {"total_requests", 0}
                    }
                );

            case MessageType::RESET_SIM:
                return makeResponse(request, MessageType::ACK,{{"status", "simulation_reset"}});

            case MessageType::BYE:
                return makeResponse(request, MessageType::ACK,{{"message", "connection_closed"}});

            default:
                return makeError(request, "Nieobsługiwany typ wiadomości.");
        }

    } catch (const exception& e) {
        return makeError(request, e.what());
    }
}

Message RequestHandler::makeResponse(Message& request, MessageType type, nlohmann::json payload) {
    return Message{type,request._message_id,time(nullptr),request._session_token, payload};
}

Message RequestHandler::makeError(Message& request, string errorMessage) {
    return Message{MessageType::ERROR,request._message_id,time(nullptr),request._session_token,{{"error", errorMessage}}};
}