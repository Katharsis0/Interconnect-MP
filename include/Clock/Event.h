#pragma once
#include <cstdint>
#include <string>

struct Event {
    uint64_t timestamp;
    int pe_id;
    std::string action;

    bool operator>(const Event& other) const {
        return timestamp > other.timestamp;
    }
};