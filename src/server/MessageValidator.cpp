#include <server/MessageValidator.h>
#include <stdexcept>
#include <string>

using namespace models;
using namespace std;

void MessageValidator::validate(Message &message) {
    if (message._message_id.empty()) {
        throw runtime_error("Brak message_id.");
    }

    if (message._timestamp <= 0) {
        throw runtime_error("Brak lub niepoprawny timestamp.");
    }

    switch (message._type) {
        case MessageType::HELLO:
            requirePayloadField(message, "client_id");
            break;

        case MessageType::AUTH:
            requirePayloadField(message, "login");
            requirePayloadField(message, "password");
            break;

        case MessageType::BYE:
        case MessageType::PING:
            break;

        case MessageType::ATTACH:
        case MessageType::DETACH:
        case MessageType::STATUS:
            requirePayloadField(message, "ue_id");
            break;

        case MessageType::GET_STATS:
        case MessageType::RESET_SIM:
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
