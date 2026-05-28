#include <server/UeSimulation.h>

#include <stdexcept>

using namespace std;

UeSimulation::UeSimulation() {
    _ues["UE_01"] = UeState::DETACHED;
    _ues["UE_02"] = UeState::DETACHED;
    _ues["UE_03"] = UeState::DETACHED;
}

string UeSimulation::stateToString(UeState state) {
    switch (state) {
        case UeState::DETACHED:
            return "DETACHED";
        case UeState::ATTACHED:
            return "ATTACHED";
        default:
            return "UNKNOWN";
    }
}

bool UeSimulation::exists(const string& ueId) const {
    return _ues.find(ueId) != _ues.end();
}

nlohmann::json UeSimulation::attach(const string& ueId) {
    if (!exists(ueId)) {
        throw runtime_error("NOT_FOUND: UE does not exist.");
    }

    if (_ues[ueId] == UeState::ATTACHED) {
        throw runtime_error("INVALID_STATE: UE is already attached.");
    }

    _ues[ueId] = UeState::ATTACHED;
    _attachCount++;

    return {
        {"status", "SUCCESS"},
        {"message", "UE attached successfully"},
        {"ue_id", ueId},
        {"ue_state", stateToString(_ues[ueId])}
    };
}

nlohmann::json UeSimulation::detach(const string& ueId) {
    if (!exists(ueId)) {
        throw runtime_error("NOT_FOUND: UE does not exist.");
    }

    if (_ues[ueId] == UeState::DETACHED) {
        throw runtime_error("INVALID_STATE: UE is already detached.");
    }

    _ues[ueId] = UeState::DETACHED;
    _detachCount++;

    return {
        {"status", "SUCCESS"},
        {"message", "UE detached successfully"},
        {"ue_id", ueId},
        {"ue_state", stateToString(_ues[ueId])}
    };
}

nlohmann::json UeSimulation::status(const string& ueId) {
    if (!exists(ueId)) {
        throw runtime_error("NOT_FOUND: UE does not exist.");
    }

    return {
        {"status", "SUCCESS"},
        {"ue_id", ueId},
        {"ue_state", stateToString(_ues[ueId])}
    };
}

nlohmann::json UeSimulation::stats() {
    int attached = 0;
    int detached = 0;

    for (const auto& ue : _ues) {
        if (ue.second == UeState::ATTACHED) {
            attached++;
        } else {
            detached++;
        }
    }

    return {
        {"status", "SUCCESS"},
        {"total_ues", static_cast<int>(_ues.size())},
        {"attached_ues", attached},
        {"detached_ues", detached},
        {"attach_operations", _attachCount},
        {"detach_operations", _detachCount},
        {"reset_operations", _resetCount}
    };
}

nlohmann::json UeSimulation::reset() {
    for (auto& ue : _ues) {
        ue.second = UeState::DETACHED;
    }

    _resetCount++;

    return {
        {"status", "SUCCESS"},
        {"message", "Simulation reset successfully"},
        {"total_ues", static_cast<int>(_ues.size())}
    };
}