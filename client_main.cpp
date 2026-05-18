#include <network/TcpSocket.h>

#include <iostream>
#include <exception>
#include <string>

#include "client/Client.h"

using namespace std;

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