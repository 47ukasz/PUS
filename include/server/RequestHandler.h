#ifndef NCP_REQUESTHANDLER_H
#define NCP_REQUESTHANDLER_H

#include <protocol/Message.h>

class RequestHandler {
    private:
        static models::Message makeResponse(models::Message& request, MessageType type, nlohmann::json payload);
        static models::Message makeError(models::Message& request, std::string errorMessage);
    public:
        static models::Message handleRequest(models::Message& request);
};

#endif //NCP_REQUESTHANDLER_H