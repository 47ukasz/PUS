#ifndef NCP_AUTHVALIDATOR_H
#define NCP_AUTHVALIDATOR_H

#include <string>
#include <map>

struct UserRecord {
    std::string salt;
    std::string passwordHash;
};

class AuthValidator {
    private:
        static std::map<std::string, UserRecord> _users;
        static std::string pbkdf2Hash(std::string& password, std::string& salt);
        static std::string bytesToHex(unsigned char* data,size_t length);

    public:
        static void loadUsers(std::string& path);
        static bool verify(std::string& login, std::string& password);
};

#endif //NCP_AUTHVALIDATOR_H