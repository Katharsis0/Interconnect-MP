#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include "Event.h"

class EventClock {
public:
    using EventHandler = std::function<void(const Event&)>;

    void add_event(const Event& e);
    void run();
    void stop();

    uint64_t now() const;

    void register_pe(int pe_id, EventHandler handler);

private:
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> event_queue;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    uint64_t current_time = 0;
    bool running_ = true;

    std::unordered_map<int, EventHandler> handlers_;
};
