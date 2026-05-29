#ifndef NCP_REQUESTHANDLER_H
#define NCP_REQUESTHANDLER_H

#include <protocol/Message.h>
#include <server/Server.h>

#include <string>
#include <vector>

class RequestHandler {
    private:
        static models::Message makeResponse(models::Message& request, MessageType type, nlohmann::json payload);
        static models::Message makeError(models::Message& request, std::string errorCode, std::string errorMessage);
        static std::string generateSessionToken();

    public:
        static std::vector<models::Message> handleRequest(models::Message& request, Session& session);
};

#endif //NCP_REQUESTHANDLER_H