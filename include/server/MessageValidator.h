#ifndef NCP_MESSAGEVALIDATOR_H
#define NCP_MESSAGEVALIDATOR_H

#include <protocol/Message.h>
#include <string>

class MessageValidator {
private:
    static void requirePayloadField(models::Message& message, const std::string& fieldName);
    static void requireSessionToken(models::Message& message);
    static void requireNonEmptyString(models::Message& message, const std::string& fieldName);

public:
    static void validate(models::Message& message);
};

#endif //NCP_MESSAGEVALIDATOR_H