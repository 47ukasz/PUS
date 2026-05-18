#include <network/TcpSocket.h>

#include <iostream>
#include <exception>
#include <string>
#include <unistd.h>

using namespace std;

int main() {
    signal(SIGCHLD, SIG_IGN);

    try {
        TcpSocket server;

        server.bindTo(8080);
        server.listenForConnections(5);

        cout << "[SERWER] Nasłuchuje na porcie 8080..." << endl;

        while (true) {
            TcpSocket client = server.acceptConnection();

            pid_t pid = fork();

            if (pid < 0) {
                throw runtime_error("Nie udało się utworzyć procesu");
            }

            if (pid == 0) {
                server.closeSocket();

                cout << "[SERWER] Klient uzyskał połączenie" << endl;
                client.printAddress("KLIENTA");
                string message = client.receiveData(4096);
                cout << "[SERWER] Otrzymano wiadomość: " << message << endl;
                string response = "PONG od serwera";

                client.sendData(response);

                cout << "[SERWER] Wysłano wiadomość" << endl;

                return 0;
            }

            client.closeSocket();
        }

    } catch (const exception& e) {
        cerr << "[SERWER] Błąd: "<< e.what() << endl;
        return 1;
    }

    return 0;
}