#ifndef NCP_MESSAGETYPE_H
#define NCP_MESSAGETYPE_H

#include <nlohmann/json.hpp>

enum class MessageType {
    HELLO,
    AUTH,
    AUTH_OK,
    AUTH_FAIL,
    ATTACH,
    DETACH,
    GET_STATS,
    RESET_SIM,
    STATUS,
    ACK,
    RESULT,
    ERROR,
    PING,
    PONG,
    BYE
};

NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {
    {MessageType::HELLO, "HELLO"},
    {MessageType::AUTH, "AUTH"},
    {MessageType::AUTH_OK, "AUTH_OK"},
    {MessageType::AUTH_FAIL, "AUTH_FAIL"},
    {MessageType::ATTACH, "ATTACH"},
    {MessageType::DETACH, "DETACH"},
    {MessageType::GET_STATS, "GET_STATS"},
    {MessageType::RESET_SIM, "RESET_SIM"},
    {MessageType::STATUS, "STATUS"},
    {MessageType::ACK, "ACK"},
    {MessageType::RESULT, "RESULT"},
    {MessageType::ERROR, "ERROR"},
    {MessageType::PING, "PING"},
    {MessageType::PONG, "PONG"},
    {MessageType::BYE, "BYE"}
})

#endif //NCP_MESSAGETYPE_H