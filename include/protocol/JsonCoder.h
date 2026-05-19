#ifndef NCP_JSONCODER_H
#define NCP_JSONCODER_H

#include <protocol/Message.h>

class JsonCoder {
    public:
        static std::string serialize(models::Message& message);
        static models::Message deserialize(std::string& rawJson);
};

#endif //NCP_JSONCODER_H