#include <server/MessageValidator.h>
#include <server/ValidationException.h>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace models;
using namespace std;

void MessageValidator::validate(Message &message) {
    if (message._message_id.empty()) {
        throw ValidationException("MISSING_FIELD", "Missing required field: message_id.");
    }

    if (message._timestamp <= 0) {
        throw ValidationException("MISSING_FIELD", "Missing or invalid field: timestamp.");
    }

    const long MAX_TIMESTAMP_DRIFT_SECONDS = 120;

    long now = static_cast<long>(time(nullptr));
    long messageTime = static_cast<long>(message._timestamp);
    long diff = labs(now - messageTime);

    if (diff > MAX_TIMESTAMP_DRIFT_SECONDS) {
        throw ValidationException(
            "INVALID_TIMESTAMP",
            "Message timestamp is outside the allowed time window."
        );
    }

    if (message._payload.is_null()) {
        throw ValidationException("MISSING_FIELD", "Missing required field: payload.");
    }

    switch (message._type) {
        case MessageType::HELLO:
            requirePayloadField(message, "client_id");
            requireNonEmptyString(message, "client_id");
            break;

        case MessageType::AUTH:
            requirePayloadField(message, "login");
            requirePayloadField(message, "password");
            requireNonEmptyString(message, "login");
            requireNonEmptyString(message, "password");
            break;

        case MessageType::ATTACH:
        case MessageType::DETACH:
        case MessageType::STATUS:
            requireSessionToken(message);
            requirePayloadField(message, "ue_id");
            requireNonEmptyString(message, "ue_id");
            break;

        case MessageType::GET_STATS:
        case MessageType::RESET_SIM:
        case MessageType::PING:
        case MessageType::BYE:
            requireSessionToken(message);
            break;

        default:
            throw ValidationException("INVALID_FORMAT", "Unsupported or invalid message type.");
    }
}

void MessageValidator::requirePayloadField(Message& message, const string& fieldName) {
    if (!message._payload.contains(fieldName)) {
        throw ValidationException("MISSING_FIELD", "Missing required payload field: " + fieldName + ".");
    }
}

void MessageValidator::requireSessionToken(Message& message) {
    if (message._session_token.empty()) {
        throw ValidationException("UNAUTHORIZED", "Missing session token.");
    }
}

void MessageValidator::requireNonEmptyString(Message& message, const string& fieldName) {
    if (!message._payload.at(fieldName).is_string()) {
        throw ValidationException("INVALID_FORMAT", "Payload field must be a string: " + fieldName + ".");
    }

    string value = message._payload.at(fieldName);

    if (value.empty()) {
        throw ValidationException("INVALID_FORMAT", "Payload field cannot be empty: " + fieldName + ".");
    }
}