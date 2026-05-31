#ifndef NCP_REQUESTHANDLER_H
#define NCP_REQUESTHANDLER_H

#include <protocol/Message.h>
#include <server/Server.h>
#include <server/UeSimulation.h>

#include <mutex>
#include <string>
#include <vector>

class RequestHandler {
private:
    static models::Message makeResponse(models::Message& request, MessageType type, nlohmann::json payload);
    static models::Message makeError(models::Message& request, std::string errorCode, std::string errorMessage);
    static std::string generateSessionToken();
    static bool isAuthorized(models::Message& request, Session& session);
    static bool isRateLimited(Session& session);

    static bool hasProcessedMessage(Session& session, models::Message& request);
    static void storeProcessedMessage(
        Session& session,
        models::Message& request,
        const std::vector<models::Message>& responses
    );

public:
    static std::vector<models::Message> handleRequest(
        models::Message& request,
        Session& session,
        UeSimulation& simulation,
        std::mutex& simulationMutex
    );
};

#endif //NCP_REQUESTHANDLER_H