#include <network/TcpSocket.h>

#include <iostream>
#include <exception>
#include <string>

using namespace std;

int main() {
    try {
        TcpSocket server;

        server.bindTo(8080);
        server.listenForConnections(5);

        cout << "[SERWER] Nasłuchuje na porcie 8080..." << endl;

        TcpSocket client = server.acceptConnection();

        cout << "[SERWER] Klient uzyskał połączenie" << endl;

        string message = client.receiveData(4096);

        cout << "[SERWER] Otrzymano wiadomość: " << message << endl;

        string response = "PONG od serwera";

        client.sendData(response);

        cout << "[SERWER] Wysłano wiadomość" << endl;

    } catch (const exception& e) {
        cerr << "[SERWER] Błąd: "<< e.what() << endl;
    }


    return 0;
}