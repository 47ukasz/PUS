#include <client/CommandMapper.h>

using namespace std;
using namespace models;

int CommandMapper::_messageCounter = 0;

vector<string> split(string& line) {
    istringstream iss(line);
    vector<string> tokens;
    string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

Message CommandMapper::mapToMessage(string cli) {
    vector<string> tokens = split(cli);

    if (tokens.empty()) {
        throw runtime_error("Nie wprowadzono nazwy komendy.");
    }

    Message message{};
    message._timestamp = time(nullptr);
    message._message_id = generateMessageId();

    string command = tokens[0];

    if (command == "connect") {
        if (tokens.size() != 3) {
            throw runtime_error("Użycie: connect <host> <port>");
        }

        message._type = MessageType::HELLO;
        message._payload = {{"host", tokens[1]},{"port", stoi(tokens[2])}};

    } else if (command == "login") {
        if (tokens.size() != 3) {
            throw runtime_error("Użycie: login <user> <password>");
        }

        message._type = MessageType::AUTH;
        message._payload = {{"login", tokens[1]}, {"password", tokens[2]}};

    } else if (command == "attach") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: attach <ue_id>");
        }

        message._type = MessageType::ATTACH;
        message._payload = { {"ue_id", tokens[1]}};

    } else if (command == "detach") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: detach <ue_id>");
        }

        message._type = MessageType::DETACH;
        message._payload = {{"ue_id", tokens[1]}};

    } else if (command == "status") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: status <ue_id>");
        }

        message._type = MessageType::STATUS;
        message._payload = {{"ue_id", tokens[1]}};

    } else if (command == "stats") {
        message._type = MessageType::GET_STATS;
        message._payload = nlohmann::json::object();

    } else if (command == "reset") {
        message._type = MessageType::RESET_SIM;
        message._payload = nlohmann::json::object();

    } else if (command == "ping") {
        message._type = MessageType::PING;
        message._payload = nlohmann::json::object();

    } else if (command == "exit") {
        message._type = MessageType::BYE;
        message._payload = nlohmann::json::object();

    } else {
        throw runtime_error("Nieznana komenda: " + command);
    }

    return message;
}

string CommandMapper::generateMessageId() {
    return to_string(++_messageCounter);
}

