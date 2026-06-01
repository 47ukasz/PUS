#include "client/Client.h"
#include <protocol/Message.h>

#include <iostream>
#include <exception>
#include <string>

using namespace std;
using namespace models;

int main() {
    try {
        Client client;

        client.run();
    } catch (const exception& e) {
        cerr << "[KLIENT] Błąd: " << e.what() << endl;
        return 1;
    }

    return 0;
}