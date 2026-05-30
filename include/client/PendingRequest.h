#ifndef NCP_PENDINGREQUEST_H
#define NCP_PENDINGREQUEST_H
#include <ctime>
#include <string>
#include "protocol/Message.h"

struct PendingRequest {
    models::Message message;
    std::string rawMessage;

    int retryCount = 0;
    bool ackReceived = false;
    bool completed = false;

    time_t lastSentAt;
};

#endif //NCP_PENDINGREQUEST_H