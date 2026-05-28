#ifndef NCP_UESIMULATION_H
#define NCP_UESIMULATION_H

#include <map>
#include <string>
#include <nlohmann/json.hpp>

enum class UeState {
    DETACHED,
    ATTACHED
};

class UeSimulation {
private:
    std::map<std::string, UeState> _ues;
    int _attachCount = 0;
    int _detachCount = 0;
    int _resetCount = 0;

    static std::string stateToString(UeState state);

public:
    UeSimulation();

    nlohmann::json attach(const std::string& ueId);
    nlohmann::json detach(const std::string& ueId);
    nlohmann::json status(const std::string& ueId);
    nlohmann::json stats();
    nlohmann::json reset();

    bool exists(const std::string& ueId) const;
};

#endif //NCP_UESIMULATION_H