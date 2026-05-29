#ifndef NCP_TIMEOUTCONFIG_H
#define NCP_TIMEOUTCONFIG_H

struct TimeoutConfig {
    int helloTimeout = 5;
    int authTimeout = 10;
    int ackTimeout = 2;
    int resultTimeout = 10;
    int idleTimeout = 30;
};

enum class TimeoutType {
    Hello,
    Auth,
    Ack,
    Result,
    Idle
};

#endif //NCP_TIMEOUTCONFIG_H