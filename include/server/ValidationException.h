#ifndef NCP_VALIDATIONEXCEPTION_H
#define NCP_VALIDATIONEXCEPTION_H

#include <stdexcept>
#include <string>

class ValidationException : public std::runtime_error {
private:
    std::string _errorCode;

public:
    ValidationException(const std::string& errorCode, const std::string& errorMessage)
        : std::runtime_error(errorMessage), _errorCode(errorCode) {}

    std::string errorCode() const {
        return _errorCode;
    }
};

#endif //NCP_VALIDATIONEXCEPTION_H