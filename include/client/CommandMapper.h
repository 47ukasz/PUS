#ifndef NCP_COMMANDMAPPER_H
#define NCP_COMMANDMAPPER_H

#include <protocol/Message.h>

class CommandMapper {
    private:
        static int _messageCounter;
        static std::string generateMessageId();
    public:
        static models::Message mapToMessage(std::string cli);
};

#endif //NCP_COMMANDMAPPER_H