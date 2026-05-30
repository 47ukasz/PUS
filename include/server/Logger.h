#ifndef NCP_LOGGER_H
#define NCP_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

class Logger {
private:
    static std::mutex _mutex;
    static std::ofstream _file;
    static bool _initialized;

    static void log(const std::string& level, const std::string& message);

public:
    static void init(const std::string& path = "logs/server.log");
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void close();
};

#endif //NCP_LOGGER_H