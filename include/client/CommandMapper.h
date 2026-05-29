#ifndef NCP_COMMANDMAPPER_H
#define NCP_COMMANDMAPPER_H

#include <protocol/Message.h>

struct ParsedCommand {
    models::Message message;
    std::string host;
    int port = 0;
};

class CommandMapper {
    private:
        static int _messageCounter;
        static std::string generateMessageId();
    public:
        static ParsedCommand parse(std::string cli);
};

#endif //NCP_COMMANDMAPPER_H