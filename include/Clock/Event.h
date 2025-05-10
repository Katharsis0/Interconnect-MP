#pragma once
#include <cstdint>
#include <string>

struct Event {
    uint64_t timestamp; // when the event fires
    int pe_id;
    std::string action;

    uint8_t qos_;

    // when the action was _scheduled_ (so we can compute duration)
    uint64_t start_time;
    std::string scheme = "fifo"; // qos


    bool operator>(const Event& other) const {

        if (scheme == "fifo") {
            return timestamp > other.timestamp;
        }

        return qos_ > other.qos_;

    }
};