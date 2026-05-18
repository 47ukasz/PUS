#include "server/Server.h"

#include <iostream>
#include <exception>

using namespace std;

int main() {
    try {
        Server server(8080, 5);

        server.start();
    }  catch (const exception& e) {
        cerr << "[SERWER] Błąd: "<< e.what() << endl;
        return 1;
    }

    return 0;
}