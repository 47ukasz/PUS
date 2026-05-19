#include "client/Client.h"
#include <network/TcpSocket.h>
#include <protocol/Message.h>
#include <protocol/JsonCoder.h>

#include <iostream>
#include <exception>
#include <string>

using namespace std;
using namespace models;

int main() {
    try {
        Client client("127.0.0.1", 8080);

        client.connect();
    } catch (const exception& e) {
        cerr << "[KLIENT] Błąd: " << e.what() << endl;
        return 1;
    }

    return 0;
}