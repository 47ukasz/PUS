#ifndef NCP_TIMEOUTMANAGER_H
#define NCP_TIMEOUTMANAGER_H
#include "TimeoutConfig.h"

class TimeoutManager {
    private:
        TimeoutConfig _config;
    public:
        TimeoutManager(const TimeoutConfig& config);
        int timeoutFor(TimeoutType type);
};

#endif //NCP_TIMEOUTMANAGER_H