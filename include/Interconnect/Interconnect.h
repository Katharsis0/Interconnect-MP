#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#pragma once

#include <queue>
#include <mutex>
#include <unordered_map>
#include "../Clock/EventClock.h"
#include "../Messages/Messages.h"

// Forward declaration
class Cache;

class Interconnect {
public:
    explicit Interconnect(EventClock* clock);

    void register_cache(uint8_t cache_id, Cache* cache);

    // Called by PEs to send a message
    void send(uint8_t src_pe, const Message& msg);

    // Called by EventClock when event happens
    void process_next();


private:

    // Simulation control (scheduling, source, message)
    struct QueuedMessage {
        Message msg; // Original Message (ReadMem, WriteResp...)
        uint8_t src_pe; // Sender
        uint64_t scheduled_time; // EventClock timestamp
    };

    std::queue<QueuedMessage> fifo_;
    std::mutex fifo_mutex_;
    EventClock* clock_;

    std::unordered_map<uint8_t, Cache*> caches_;

    // Helpers
    uint64_t getLatencyForMessage(const Message& msg);
    uint8_t getMessageDestination(const Message& msg);
};

#endif // INTERCONNECT_H
