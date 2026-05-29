#include <client/CommandMapper.h>

#include <ctime>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

ParsedCommand CommandMapper::parse(string cli) {
    vector<string> tokens = split(cli);

    if (tokens.empty()) {
        throw runtime_error("Nie wprowadzono nazwy komendy.");
    }

    ParsedCommand result{};
    result.message._timestamp = time(nullptr);
    result.message._message_id = generateMessageId();

    string command = tokens[0];

    if (command == "connect") {
        if (tokens.size() != 3) {
            throw runtime_error("Użycie: connect <host> <port>");
        }

        result.host = tokens[1];
        result.port = stoi(tokens[2]);

        if (result.port <= 0) {
            throw runtime_error("Użycie: connect <host> <port>");
        }

        result.message._type = MessageType::HELLO;
        result.message._payload = {{"client_id", "client_01"}};

        return result;
    }

    if (command == "login") {
        if (tokens.size() != 3) {
            throw runtime_error("Użycie: login <user> <password>");
        }

        result.message._type = MessageType::AUTH;
        result.message._payload = {{"login", tokens[1]}, {"password", tokens[2]}};

    } else if (command == "attach") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: attach <ue_id>");
        }

        result.message._type = MessageType::ATTACH;
        result.message._payload = { {"ue_id", tokens[1]}};

    } else if (command == "detach") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: detach <ue_id>");
        }

        result.message._type = MessageType::DETACH;
        result.message._payload = {{"ue_id", tokens[1]}};

    } else if (command == "status") {
        if (tokens.size() != 2) {
            throw runtime_error("Użycie: status <ue_id>");
        }

        result.message._type = MessageType::STATUS;
        result.message._payload = {{"ue_id", tokens[1]}};

    } else if (command == "stats") {
        result.message._type = MessageType::GET_STATS;
        result.message._payload = nlohmann::json::object();

    } else if (command == "reset") {
        result.message._type = MessageType::RESET_SIM;
        result.message._payload = nlohmann::json::object();

    } else if (command == "ping") {
        result.message._type = MessageType::PING;
        result.message._payload = nlohmann::json::object();

    } else if (command == "exit") {
        result.message._type = MessageType::BYE;
        result.message._payload = nlohmann::json::object();

    } else {
        throw runtime_error("Nieznana komenda: " + command);
    }

    return result;
}

string CommandMapper::generateMessageId() {
    return to_string(++_messageCounter);
}

