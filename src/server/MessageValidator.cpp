#include <server/MessageValidator.h>

using namespace models;
using namespace std;

void MessageValidator::validate(Message &message) {
    if (message._message_id.empty()) {
        throw runtime_error("Brak message_id.");
    }

    switch (message._type) {
        case MessageType::AUTH:
            requirePayloadField(message, "login");
            requirePayloadField(message, "password");
            break;

        case MessageType::ATTACH:
        case MessageType::DETACH:
        case MessageType::STATUS:
            requirePayloadField(message, "ue_id");
            // requirePayloadField(message, "session_token");
            break;

        case MessageType::PING:
        case MessageType::GET_STATS:
        case MessageType::RESET_SIM:
        case MessageType::BYE:
            // requirePayloadField(message, "session_token");
            break;

        default:
            throw runtime_error("Nieobsługiwany typ wiadomości.");

    }
}

void MessageValidator::requirePayloadField(Message &message, string field) {
    if (!message._payload.contains(field)) {
        throw runtime_error("Brak wymaganego pola payload: " + field);
    }
}
