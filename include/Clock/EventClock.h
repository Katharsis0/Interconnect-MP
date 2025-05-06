#ifndef EVENTCLOCK_H
#define EVENTCLOCK_H

#pragma once

#include "../Messages/Messages.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <atomic>

// Forward declare Interconnect
class Interconnect;

// Event structure
struct Event {
    uint64_t timestamp;
    int pe_id;
    std::string action;

    // Priority queue: smallest timestamp first
    bool operator<(const Event& other) const {
        return timestamp > other.timestamp;
    }
};

// TBD
enum class ClockMode {
    Running,
    Stepping
};

class EventClock {
public:
    EventClock(ClockMode mode = ClockMode::Running);

    // Link Interconnect to EventClock
    void set_interconnect(Interconnect* interconnect);

    // Add event to event queue
    void add_event(const Event& e);

    // Run until no events or stop
    void run();

    // Run single step
    bool run_step();

    // Stop simulation
    void stop();

    // Current simulation time
    uint64_t now() const;

    // PE registration
    using EventHandler = std::function<void(const Event&)>;
    void register_pe(int pe_id, EventHandler handler);

    // PE termination notification
    void set_total_pes(int n);
    void notify_pe_finished(int pe_id);
    void wait_until_all_pes_finished();

private:
    ClockMode mode_ = ClockMode::Running;

    // Interconnect pointer
    Interconnect* interconnect_ = nullptr;

    // Priority queue of events
    std::priority_queue<Event> event_queue;
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    // PE handlers
    std::unordered_map<int, EventHandler> handlers_;

    // PE termination sync
    int total_pes = 0;
    int pes_finished = 0;
    std::mutex finish_mutex_;
    std::condition_variable finish_cv_;

    // Clock state
    std::atomic<bool> running_{true};
    uint64_t current_time = 0;
};

#endif // EVENTCLOCK_H
