#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#pragma once

#include <queue>
#include <mutex>
#include <unordered_map>
#include "../Messages/Messages.h"
#include "../RAM/FileMemory.h"

// Forward declarations
class Cache;
class EventClock;

class Interconnect {

public:
    Interconnect();

    void register_cache(uint8_t cache_id, Cache* cache);
    void send(uint8_t src_pe, const Message& msg);
    void process_next();
    void sendMessage(const Message& msg);
    void receiveMessage(const Message& msg);

    void set_clock(EventClock* clock) { clock_ = clock; }


    uint64_t getLatencyForMessage(const Message& msg);
    uint8_t getMessageDestination(const Message& msg);


private:
    FileMemory memory;

    std::string scheme = "fifont"; // qos
    std::queue<Message> fifo_;
    std::mutex fifo_mutex_;
    EventClock* clock_;

    std::unordered_map<uint8_t, Cache*> caches_;

};

#endif // INTERCONNECT_H
