#ifndef NCP_MESSAGE_H
#define NCP_MESSAGE_H
#include <string>
#include <nlohmann/json.hpp>

#include "MessageType.h"

namespace models {
    struct Message {
        MessageType _type;
        std::string _message_id;
        time_t _timestamp;
        std::string _session_token;
        nlohmann::json _payload;
    };

    inline void to_json(nlohmann::json &j, const Message &m) {
        j = {
            {"type", m._type},
            {"message_id", m._message_id},
            {"timestamp", m._timestamp},
            {"payload", m._payload}
        };

        if (!m._session_token.empty()) {
            j["session_token"] = m._session_token;
        }
    }

    inline void from_json(const nlohmann::json &j, Message &m) {
    j.at("type").get_to(m._type);
    j.at("message_id").get_to(m._message_id);
    j.at("timestamp").get_to(m._timestamp);

    if (j.contains("session_token") && !j.at("session_token").is_null()) {
        j.at("session_token").get_to(m._session_token);
    } else {
        m._session_token = "";
    }

    if (j.contains("payload") && !j.at("payload").is_null()) {
        j.at("payload").get_to(m._payload);
    } else {
        m._payload = nlohmann::json::object();
    }
}
}

#endif //NCP_MESSAGE_H