#include "protocol/TimeoutManager.h"

TimeoutManager::TimeoutManager(const TimeoutConfig& config) : _config(config) {}

int TimeoutManager::timeoutFor(TimeoutType type) {
    switch (type) {
        case TimeoutType::Hello:
            return _config.helloTimeout;
        case TimeoutType::Auth:
            return _config.authTimeout;
        case TimeoutType::Ack:
            return _config.ackTimeout;
        case TimeoutType::Result:
            return _config.resultTimeout;
        case TimeoutType::Idle:
            return _config.idleTimeout;
    }

    return _config.idleTimeout;
}