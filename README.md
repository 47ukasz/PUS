# NetControl Protocol - aplikacja klient-serwer NCP

## 1. Opis projektu

Projekt stanowi implementację aplikacji klient-serwer wykorzystującej autorski protokół aplikacyjny **NetControl Protocol - NCP**. Celem aplikacji jest umożliwienie bezpiecznego sterowania uproszczoną symulacją elementów sieciowych znajdującą się po stronie serwera.

Aplikacja została przygotowana zgodnie z dokumentacją protokołu NCP oraz dokumentacją aplikacji. Komunikacja odbywa się pomiędzy klientem CLI a serwerem przez połączenie **TCP/TLS**, a dane przesyłane są w postaci komunikatów **JSON** zgodnych ze strukturą protokołu NCP.

W projekcie zaimplementowano między innymi:

* zestawianie szyfrowanego połączenia TLS,
* inicjalizację sesji przez komunikat `HELLO`,
* uwierzytelnianie użytkownika przez `AUTH`,
* obsługę `session_token`,
* operacje administracyjne na symulowanych urządzeniach UE,
* odpowiedzi `ACK`, `RESULT` i `ERROR`,
* walidację komunikatów,
* timeouty, retransmisje i wykrywanie duplikatów `message_id`,
* mechanizm keep-alive `PING/PONG`,
* obsługę wielu klientów przez wątki,
* wspólny stan symulacji po stronie serwera,
* moduł logowania diagnostycznego.

Projekt jest demonstracyjną implementacją protokołu NCP i pokazuje pełny przepływ komunikacji opisany w dokumentacji.

---

## 2. Technologie

Projekt został wykonany w języku **C++** z wykorzystaniem:

* **CMake** - budowanie projektu,
* **OpenSSL** - obsługa połączeń TLS,
* **nlohmann/json** - serializacja i deserializacja komunikatów JSON,
* **TCP sockets** - komunikacja sieciowa klient-serwer,
* **std::thread** - obsługa wielu klientów oraz mechanizmów pomocniczych,
* **mutex / atomic** - synchronizacja dostępu do wspólnych zasobów.

---

## 3. Architektura aplikacji

Aplikacja składa się z dwóch głównych części:

```text
Klient CLI  <---- TCP/TLS + JSON/NCP ---->  Serwer NCP
```

### Klient

Klient działa jako aplikacja konsolowa. Użytkownik wpisuje komendy tekstowe, które są mapowane na komunikaty protokołu NCP.

Klient odpowiada za:

* zestawienie połączenia z serwerem,
* wysyłanie komunikatów NCP,
* zapamiętanie `session_token` po poprawnym logowaniu,
* automatyczne dołączanie tokenu do kolejnych żądań,
* odbieranie odpowiedzi serwera,
* retransmisję komunikatów po timeoutach,
* obsługę `PING/PONG`,
* zakończenie sesji przez `BYE`.

### Serwer

Serwer nasłuchuje na porcie `8080`, przyjmuje połączenia klientów i obsługuje je w osobnych wątkach. Stan symulacji UE jest wspólny dla całego serwera i zabezpieczony mutexem.

Serwer odpowiada za:

* obsługę sesji klienta,
* walidację komunikatów,
* uwierzytelnianie użytkownika,
* generowanie tokenu sesji,
* obsługę operacji administracyjnych,
* kontrolę timeoutów `HELLO` i `AUTH`,
* mechanizm keep-alive,
* wykrywanie duplikatów `message_id`,
* obsługę błędów,
* logowanie diagnostyczne.

---

## 4. Struktura komunikatów NCP

Komunikaty NCP przesyłane są jako JSON. Podstawowa struktura wiadomości:

```json
{
  "type": "ATTACH",
  "message_id": "4",
  "timestamp": 1780167251,
  "session_token": "token_xxx",
  "payload": {
    "ue_id": "UE_01"
  }
}
```

Znaczenie pól:

* `type` - typ komunikatu NCP,
* `message_id` - unikalny identyfikator wiadomości w ramach sesji,
* `timestamp` - czas utworzenia komunikatu,
* `session_token` - token aktywnej sesji, wymagany po uwierzytelnieniu,
* `payload` - dane właściwe dla danego typu komunikatu.

---

## 5. Zaimplementowane typy komunikatów

W projekcie zaimplementowano następujące typy komunikatów zgodne z dokumentacją NCP:

```text
HELLO
AUTH
AUTH_OK
AUTH_FAIL
ATTACH
DETACH
GET_STATS
RESET_SIM
STATUS
ACK
RESULT
ERROR
PING
PONG
BYE
```

Przykładowy przepływ sesji:

```text
HELLO      -> ACK
AUTH       -> AUTH_OK
ATTACH     -> ACK + RESULT
STATUS     -> RESULT
DETACH     -> ACK + RESULT
GET_STATS  -> RESULT
RESET_SIM  -> ACK + RESULT
BYE        -> ACK
```

---

## 6. Obsługiwane komendy klienta

Po uruchomieniu klienta dostępne są następujące komendy:

```text
connect <host> <port>
login <user> <password>
attach <ue_id>
detach <ue_id>
status <ue_id>
stats
reset
ping
exit
```

Przykład:

```text
connect 127.0.0.1 8080
login admin admin
status UE_01
attach UE_01
status UE_01
detach UE_01
stats
reset
exit
```

---

## 7. Symulacja UE

Serwer przechowuje uproszczony model symulowanych urządzeń UE. Domyślnie dostępne są:

```text
UE_01
UE_02
UE_03
```

Każde UE może znajdować się w jednym z dwóch stanów:

```text
DETACHED
ATTACHED
```

Obsługiwane operacje:

### `ATTACH`

Podłącza UE do sieci.

Przykład:

```text
attach UE_01
```

Odpowiedź:

```text
ACK PROCESSING
RESULT SUCCESS
```

### `DETACH`

Odłącza UE od sieci.

Przykład:

```text
detach UE_01
```

Odpowiedź:

```text
ACK PROCESSING
RESULT SUCCESS
```

### `STATUS`

Zwraca aktualny stan wskazanego UE.

Przykład:

```text
status UE_01
```

Odpowiedź może zawierać:

```json
{
  "status": "SUCCESS",
  "ue_id": "UE_01",
  "ue_state": "ATTACHED"
}
```

### `GET_STATS`

Zwraca statystyki symulacji.

Przykład:

```text
stats
```

Odpowiedź może zawierać:

```json
{
  "status": "SUCCESS",
  "total_ues": 3,
  "attached_ues": 1,
  "detached_ues": 2,
  "attach_operations": 1,
  "detach_operations": 0,
  "reset_operations": 0
}
```

### `RESET_SIM`

Resetuje symulację i przywraca wszystkie UE do stanu `DETACHED`.

Przykład:

```text
reset
```

---

## 8. Bezpieczeństwo i sesja

Projekt realizuje główne założenia bezpieczeństwa z dokumentacji protokołu NCP.

Zaimplementowano:

* komunikację przez TCP/TLS,
* uwierzytelnianie użytkownika przez `AUTH`,
* odpowiedzi `AUTH_OK` i `AUTH_FAIL`,
* generowanie losowego `session_token`,
* wymaganie tokenu przy operacjach po zalogowaniu,
* walidację poprawności tokenu,
* odrzucanie operacji bez aktywnej sesji,
* maskowanie danych wrażliwych w logach.

Dane testowe użytkownika:

```text
login: admin
hasło: admin
```

Po poprawnym logowaniu serwer zwraca token sesji, który klient zapisuje i dołącza do kolejnych komunikatów.

---

## 9. Walidacja i obsługa błędów

Serwer waliduje strukturę komunikatów NCP i odrzuca błędne żądania.

Sprawdzane są między innymi:

* obecność `message_id`,
* poprawność `timestamp`,
* świeżość `timestamp`,
* obecność `payload`,
* obecność wymaganych pól w `payload`,
* obecność `session_token` po uwierzytelnieniu,
* poprawność stanu sesji,
* istnienie wskazanego UE,
* poprawność przejścia stanu UE.

Błędy zwracane są jako komunikaty `ERROR`.

Przykład:

```json
{
  "type": "ERROR",
  "message_id": "5",
  "timestamp": 1780167259,
  "payload": {
    "error_code": "INVALID_STATE",
    "error_message": "UE is already attached."
  }
}
```

Obsługiwane kody błędów obejmują między innymi:

```text
MISSING_FIELD
INVALID_FORMAT
INVALID_TIMESTAMP
UNAUTHORIZED
INVALID_STATE
NOT_FOUND
RATE_LIMIT_EXCEEDED
INTERNAL_ERROR
```

---

## 10. ACK + RESULT

Dla operacji zmieniających stan serwer zwraca dwie odpowiedzi:

```text
ACK PROCESSING
RESULT SUCCESS
```

albo:

```text
ACK PROCESSING
ERROR
```

Dotyczy to operacji:

```text
ATTACH
DETACH
RESET_SIM
```

Przykład:

```text
attach UE_01
```

Odpowiedzi:

```json
{
  "type": "ACK",
  "payload": {
    "status": "PROCESSING"
  }
}
```

```json
{
  "type": "RESULT",
  "payload": {
    "status": "SUCCESS",
    "message": "UE attached successfully",
    "ue_id": "UE_01",
    "ue_state": "ATTACHED"
  }
}
```

Dla operacji odczytu, takich jak `STATUS` i `GET_STATS`, serwer zwraca pojedynczy `RESULT`.

---

## 11. Timeouty i retransmisje

Klient obsługuje timeouty i retransmisję komunikatów.

Zaimplementowano:

* oczekiwanie na `ACK`,
* oczekiwanie na `RESULT`,
* ponowienie wysłania komunikatu po timeoutcie,
* maksymalną liczbę retransmisji,
* użycie tego samego `message_id` przy retransmisji.

Dzięki temu mechanizm retransmisji jest zgodny z założeniem protokołu NCP: ponawiana wiadomość ma ten sam `message_id`, a serwer nie wykonuje operacji drugi raz, tylko zwraca zapamiętaną odpowiedź.

---

## 12. Duplikaty `message_id`

Serwer zapamiętuje obsłużone `message_id` w ramach sesji.

Jeśli otrzyma ponownie komunikat z tym samym `message_id`, nie wykonuje operacji ponownie. Zamiast tego zwraca poprzednio zapamiętaną odpowiedź.

Przykład:

```text
attach UE_01 z message_id = 4 -> ACK + RESULT SUCCESS
attach UE_01 z message_id = 4 ponownie -> poprzedni ACK + RESULT SUCCESS
```

Dzięki temu operacje zmieniające stan są bezpieczne przy retransmisji.

---

## 13. Keep-alive

Serwer posiada mechanizm keep-alive.

Jeżeli aktywna, zalogowana sesja jest bezczynna przez określony czas, serwer wysyła komunikat:

```text
PING
```

Klient automatycznie odpowiada:

```text
PONG
```

Jeżeli serwer nie otrzyma odpowiedzi `PONG` po kilku próbach, zamyka sesję.

Dodatkowo użytkownik może ręcznie wysłać komendę:

```text
ping
```

w celu sprawdzenia aktywności połączenia.

---

## 14. Rate limiting

Serwer posiada prosty mechanizm ograniczania liczby żądań w ramach sesji.

Jeżeli klient przekroczy dopuszczalną liczbę żądań w oknie czasowym, serwer zwraca:

```json
{
  "type": "ERROR",
  "payload": {
    "error_code": "RATE_LIMIT_EXCEEDED",
    "error_message": "Too many requests in the current time window."
  }
}
```

Mechanizm ten zabezpiecza serwer przed nadmierną liczbą żądań od jednego klienta.

---

## 15. Logowanie diagnostyczne

Projekt posiada moduł logowania diagnostycznego po stronie serwera.

Logi zapisywane są do pliku:

```text
logs/server.log
```

oraz wypisywane w konsoli serwera.

Logowane są między innymi:

* uruchomienie serwera,
* połączenia klientów,
* odebrane komunikaty NCP,
* wysłane odpowiedzi,
* poprawne i błędne logowania,
* operacje na UE,
* duplikaty `message_id`,
* zdarzenia keep-alive,
* błędy,
* zamknięcia sesji.

Dane wrażliwe, takie jak hasło i token sesji, są maskowane w logach.

Przykład:

```text
[2026-05-30 20:54:02][INFO] AUTH_OK dla użytkownika: admin
[2026-05-30 20:54:11][INFO] ATTACH wykonano dla UE_01.
[2026-05-30 20:54:28][INFO] BYE odebrano. Sesja zostanie zakończona.
```

---

## 16. Obsługa wielu klientów

Serwer obsługuje wielu klientów przy użyciu wątków.

Każdy klient posiada własną sesję, ale stan symulacji UE jest wspólny po stronie serwera.

Przykład:

```text
Klient 1:
attach UE_01

Klient 2:
status UE_01
```

Klient 2 zobaczy stan:

```text
UE_01 -> ATTACHED
```

Dostęp do wspólnego stanu symulacji jest zabezpieczony mutexem.

---

## 17. Wymagania

Do uruchomienia projektu wymagane są:

* system Linux,
* kompilator C++ obsługujący C++17,
* CMake,
* OpenSSL,
* dostęp do terminala.

Przykładowa instalacja zależności:

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev openssl
```

---

## 18. Certyfikaty TLS

Projekt wykorzystuje TLS do zabezpieczenia komunikacji między klientem a serwerem. Serwer korzysta z certyfikatu oraz klucza prywatnego, natomiast klient weryfikuje certyfikat serwera na podstawie lokalnego certyfikatu CA.

Katalog `certs` jest ignorowany przez `.gitignore`, dlatego po sklonowaniu lub pobraniu projektu z GitHuba należy wygenerować certyfikaty lokalnie.

W katalogu głównym projektu powinien znajdować się katalog:

```text
certs/
```

Wymagane pliki:

```text
certs/rootCA.crt
certs/rootCA.key
certs/server.crt
certs/server.key
certs/domain.ext
```

Znaczenie plików:

```text
rootCA.crt   - certyfikat lokalnego CA, używany przez klienta do weryfikacji certyfikatu serwera
rootCA.key   - klucz prywatny lokalnego CA, używany do podpisania certyfikatu serwera
server.crt   - certyfikat serwera używany przez aplikację serwera
server.key   - klucz prywatny serwera
domain.ext   - plik rozszerzeń certyfikatu serwera
```

### 18.1 Generowanie certyfikatów

Z poziomu katalogu głównego projektu należy utworzyć katalog `certs`:

```bash
mkdir -p certs
cd certs
```

Następnie należy wygenerować lokalne CA:

```bash
openssl req -x509 -sha256 -days 1825 \
  -newkey rsa:2048 \
  -nodes \
  -keyout rootCA.key \
  -out rootCA.crt
```

Podczas generowania można podać dowolne dane testowe. Przykładowo:

```text
Country Name: PL
State or Province Name: Malopolskie
Locality Name: Krakow
Organization Name: NCP
Organizational Unit Name: Project
Common Name: NCP Local CA
```

Następnie należy wygenerować klucz prywatny serwera:

```bash
openssl genrsa -out server.key 2048
```

Na podstawie klucza serwera należy wygenerować żądanie podpisania certyfikatu:

```bash
openssl req -new \
  -key server.key \
  -out server.csr
```

Podczas generowania można podać przykładowe dane:

```text
Country Name: PL
State or Province Name: Malopolskie
Locality Name: Krakow
Organization Name: NCP
Organizational Unit Name: Server
Common Name: localhost
```

Następnie należy utworzyć plik rozszerzeń certyfikatu serwera:

```bash
cat > domain.ext <<EOF
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage=digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
IP.1 = 127.0.0.1
EOF
```

Na końcu należy podpisać certyfikat serwera lokalnym CA:

```bash
openssl x509 -req \
  -CA rootCA.crt \
  -CAkey rootCA.key \
  -in server.csr \
  -out server.crt \
  -days 365 \
  -CAcreateserial \
  -extfile domain.ext
```

Po wykonaniu tych poleceń w katalogu `certs` powinny znajdować się między innymi:

```text
rootCA.crt
rootCA.key
server.key
server.csr
server.crt
domain.ext
rootCA.srl
```

Do działania aplikacji wymagane są przede wszystkim:

```text
rootCA.crt
server.crt
server.key
domain.ext
```

Pliki `server.csr` oraz `rootCA.srl` są plikami pomocniczymi i nie są wymagane do uruchomienia aplikacji.

### 18.2 Przeładowanie CMake

Po wygenerowaniu certyfikatów należy wrócić do katalogu głównego projektu:

```bash
cd ..
```

Następnie należy ponownie skonfigurować i zbudować projekt, aby katalog `certs` został skopiowany do katalogu build:

```bash
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

Jeżeli projekt jest uruchamiany w CLion, należy wykonać opcję:

```text
Reload CMake Project
```

albo usunąć katalog `cmake-build-debug` i pozwolić środowisku wygenerować go ponownie.

### 18.3 Sprawdzenie certyfikatów po zbudowaniu projektu

Po zbudowaniu projektu w katalogu `build` powinien znajdować się katalog:

```text
build/certs/
```

Można to sprawdzić poleceniem:

```bash
ls certs
```

W katalogu powinny być widoczne między innymi:

```text
rootCA.crt
server.crt
server.key
domain.ext
```

Serwer korzysta z plików:

```text
certs/server.crt
certs/server.key
```

Klient korzysta z pliku:

```text
certs/rootCA.crt
```

Oznacza to, że klient weryfikuje certyfikat serwera na podstawie lokalnego certyfikatu CA.

### 18.4 Uruchomienie po wygenerowaniu certyfikatów

Po poprawnym wygenerowaniu certyfikatów i zbudowaniu projektu można uruchomić serwer:

```bash
./ncp_server
```

W drugim terminalu można uruchomić klienta:

```bash
./ncp_client
```

Przykładowa sesja:

```text
connect 127.0.0.1 8080
login admin admin
status UE_01
attach UE_01
status UE_01
detach UE_01
stats
reset
exit
```

### 18.5 Uwaga

Certyfikaty generowane powyższymi poleceniami są certyfikatami testowymi przeznaczonymi do lokalnego uruchomienia projektu. Nie należy traktować ich jako certyfikatów produkcyjnych.

W środowisku produkcyjnym należałoby użyć certyfikatów wystawionych przez zaufany urząd certyfikacji albo odpowiednio zarządzaną infrastrukturę PKI.

---

## 19. Budowanie projektu

W katalogu głównym projektu:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

Po poprawnym zbudowaniu powinny powstać pliki wykonywalne:

```text
ncp_server
ncp_client
```

---

## 20. Uruchomienie

### Terminal 1 - serwer

```bash
cd build
./ncp_server
```

Serwer uruchamia nasłuchiwanie na porcie:

```text
8080
```

### Terminal 2 - klient

```bash
cd build
./ncp_client
```

Następnie w kliencie można wykonać przykładową sesję:

```text
connect 127.0.0.1 8080
login admin admin
status UE_01
attach UE_01
status UE_01
detach UE_01
stats
reset
exit
```

---

## 21. Przykładowy scenariusz działania

```text
connect 127.0.0.1 8080
```

Klient zestawia szyfrowane połączenie TLS i wysyła `HELLO`.

```text
login admin admin
```

Serwer uwierzytelnia użytkownika i zwraca `AUTH_OK` z tokenem sesji.

```text
status UE_01
```

Serwer zwraca aktualny stan UE.

```text
attach UE_01
```

Serwer zwraca `ACK PROCESSING`, wykonuje operację i zwraca `RESULT SUCCESS`.

```text
detach UE_01
```

Serwer odłącza UE i zwraca `ACK + RESULT`.

```text
stats
```

Serwer zwraca statystyki symulacji.

```text
reset
```

Serwer resetuje stan symulacji.

```text
exit
```

Klient wysyła `BYE`, serwer zwraca `ACK SESSION_CLOSED`, a klient kończy działanie.

---

## 22. Scenariusze testowe

### 22.1 Poprawne logowanie

```text
connect 127.0.0.1 8080
login admin admin
```

Oczekiwany wynik:

```text
AUTH_OK
session_token zapisany po stronie klienta
```

### 22.2 Błędne logowanie

```text
connect 127.0.0.1 8080
login admin zlehaslo
```

Oczekiwany wynik:

```text
AUTH_FAIL
```

### 22.3 Operacja bez logowania

```text
connect 127.0.0.1 8080
status UE_01
```

Oczekiwany wynik:

```text
ERROR UNAUTHORIZED
```

### 22.4 Podłączenie UE

```text
connect 127.0.0.1 8080
login admin admin
attach UE_01
```

Oczekiwany wynik:

```text
ACK PROCESSING
RESULT SUCCESS
UE_01 -> ATTACHED
```

### 22.5 Ponowne podłączenie tego samego UE

```text
attach UE_01
attach UE_01
```

Oczekiwany wynik drugiej operacji:

```text
ACK PROCESSING
ERROR INVALID_STATE
```

### 22.6 Odłączenie UE

```text
detach UE_01
```

Oczekiwany wynik:

```text
ACK PROCESSING
RESULT SUCCESS
UE_01 -> DETACHED
```

### 22.7 Nieistniejące UE

```text
status UE_99
```

Oczekiwany wynik:

```text
ERROR NOT_FOUND
```

### 22.8 Reset symulacji

```text
attach UE_01
attach UE_02
reset
stats
```

Oczekiwany wynik:

```text
wszystkie UE wracają do stanu DETACHED
```

### 22.9 Wspólny stan dla wielu klientów

Klient 1:

```text
connect 127.0.0.1 8080
login admin admin
attach UE_01
```

Klient 2:

```text
connect 127.0.0.1 8080
login admin admin
status UE_01
```

Oczekiwany wynik:

```text
Klient 2 widzi UE_01 jako ATTACHED
```

### 22.10 Keep-alive

Po zalogowaniu należy pozostawić klienta bezczynnego.

Oczekiwany wynik:

```text
serwer wysyła PING
klient automatycznie odpowiada PONG
sesja pozostaje aktywna
```

### 22.11 Timeout AUTH

Po wykonaniu:

```text
connect 127.0.0.1 8080
```

nie należy wpisywać `login` przez określony czas.

Oczekiwany wynik:

```text
serwer zamyka sesję po przekroczeniu timeoutu AUTH
klient może ponownie wykonać connect
```

### 22.12 Duplikat message_id

W przypadku ponownego przesłania komunikatu z tym samym `message_id` serwer powinien zwrócić poprzednią odpowiedź bez ponownego wykonania operacji.

Oczekiwany wynik:

```text
ten sam message_id -> ta sama poprzednia odpowiedź
operacja nie jest wykonywana drugi raz
```

---

## 23. Podsumowanie

Projekt realizuje główne założenia dokumentacji protokołu NCP oraz dokumentacji aplikacji. Zaimplementowano kompletny przepływ komunikacji klient-serwer, szyfrowaną transmisję TLS, komunikaty JSON, sesję z tokenem, uwierzytelnianie, obsługę operacji administracyjnych, walidację, obsługę błędów, timeouty, retransmisje, duplikaty `message_id`, keep-alive, rate limiting, logowanie diagnostyczne oraz współdzielony stan symulacji UE dla wielu klientów.

Aplikacja pokazuje praktyczne działanie autorskiego protokołu aplikacyjnego i pozwala przetestować najważniejsze scenariusze opisane w dokumentacji.