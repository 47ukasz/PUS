#include <server/Logger.h>

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

mutex Logger::_mutex;
ofstream Logger::_file;
bool Logger::_initialized = false;

void Logger::init(const string& path) {
    lock_guard<mutex> lock(_mutex);

    if (_initialized) {
        return;
    }

    filesystem::create_directories("logs");

    _file.open(path, ios::app);

    if (!_file.is_open()) {
        cerr << "[LOGGER] Nie udało się otworzyć pliku logów: " << path << endl;
    }

    _initialized = true;
}

void Logger::info(const string& message) {
    log("INFO", message);
}

void Logger::warn(const string& message) {
    log("WARN", message);
}

void Logger::error(const string& message) {
    log("ERROR", message);
}

void Logger::log(const string& level, const string& message) {
    lock_guard<mutex> lock(_mutex);

    if (!_initialized) {
        filesystem::create_directories("logs");
        _file.open("logs/server.log", ios::app);
        _initialized = true;
    }

    time_t now = time(nullptr);
    tm localTime = *localtime(&now);

    stringstream line;
    line << "[" << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "]"
         << "[" << level << "] "
         << message;

    cout << line.str() << endl;

    if (_file.is_open()) {
        _file << line.str() << endl;
        _file.flush();
    }
}

void Logger::close() {
    lock_guard<mutex> lock(_mutex);

    if (_file.is_open()) {
        _file.close();
    }

    _initialized = false;
}