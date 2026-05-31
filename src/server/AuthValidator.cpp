#include <server/AuthValidator.h>

#include <openssl/evp.h>

#include <nlohmann/json.hpp>

#include <fstream>

#include <sstream>

#include <iomanip>
#include <iostream>

#include <stdexcept>

using namespace std;
using json = nlohmann::json;

map<string, UserRecord> AuthValidator::_users;

void AuthValidator::loadUsers(string& path) {
    ifstream file(path);

    if (!file.is_open()) {
        throw runtime_error("Nie udało się otworzyć pliku users.json");
    }

    json config;
    file >> config;
    _users.clear();

    for (const auto& user : config["users"]) {
        string login = user.at("login");
        string salt = user.at("salt");

        string passwordHash = user.at("password_hash");
        _users[login] = {
            salt,
            passwordHash
        };
    }
}

bool AuthValidator::verify(string& login, string& password) {
    auto user = _users.find(login);

    if (user == _users.end()) {
        return false;
    }

    string calculatedHash = pbkdf2Hash(
        password,
        user->second.salt
    );

    return calculatedHash == user->second.passwordHash;
}

string AuthValidator::pbkdf2Hash(string& password, string& salt) {
    int ITERATIONS = 100000;
    int HASH_LENGTH = 32;

    unsigned char hash[HASH_LENGTH];

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        reinterpret_cast<const unsigned char*>(salt.data()),
        static_cast<int>(salt.size()),
        ITERATIONS,
        EVP_sha256(),
        HASH_LENGTH,
        hash
    );

    if (result != 1) {
        throw runtime_error("Nie udało się obliczyć hash hasła.");
    }

    return bytesToHex(hash, HASH_LENGTH);
}

string AuthValidator::bytesToHex(unsigned char* data, size_t length) {
    stringstream ss;

    for (size_t i = 0; i < length; i++) {
        ss << hex << setw(2) << setfill('0') << static_cast<int>(data[i]);
    }

    return ss.str();
}