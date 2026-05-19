#include <protocol/JsonCoder.h>

using namespace std;
using namespace models;
using namespace nlohmann;

string JsonCoder::serialize(Message &message) {
    json j = message;

    return j.dump();
}

Message JsonCoder::deserialize(string &rawJson) {
    try {
        json j = json::parse(rawJson);

        return j.get<Message>();
    } catch (exception &e) {
        throw runtime_error(string("Błąd dekodowania wiadomości JSON ") + e.what());
    }
}
