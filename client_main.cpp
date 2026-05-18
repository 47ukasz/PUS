#include <network/TcpSocket.h>

#include <iostream>
#include <exception>
#include <string>

using namespace std;

int main() {
    try {
        TcpSocket client;

        string host = "127.0.0.1";
        client.connectTo(host, 8080);

        cout << "[KLIENT] Uzyskano połączenie z serwerem" << endl;

        string message = "PING od klienta";
        client.sendData(message);

        cout << "[KLIENT] Wysłano wiadomość" << endl;

        string response = client.receiveData(4096);

        cout << "[KLIENT] Otrzymano wiadomość: " << response << endl;

    } catch (const exception& e) {
        cerr << "[KLIENT] Błąd: " << e.what() << endl;
        return 1;
    }

    return 0;
}