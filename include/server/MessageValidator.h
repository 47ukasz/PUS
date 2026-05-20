#ifndef NCP_MESSAGEVALIDATOR_H
#define NCP_MESSAGEVALIDATOR_H

#include <protocol/Message.h>

class MessageValidator {
    private:
        static void requirePayloadField(models::Message &message, std::string field);
    public:
        static void validate(models::Message &message);

};

#endif //NCP_MESSAGEVALIDATOR_H