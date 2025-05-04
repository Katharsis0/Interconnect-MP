#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include "Event.h"

enum class ClockMode {
    Automatic,
    Stepping
};

class EventClock {
public:
    using EventHandler = std::function<void(const Event&)>;

    EventClock(ClockMode mode = ClockMode::Automatic);

    void add_event(const Event& e);
    void run();
    bool run_step();
    void stop();

    uint64_t now() const;

    void register_pe(int pe_id, EventHandler handler);

    // Nuevo para notificar fin de PE
    void notify_pe_finished(int pe_id);
    void wait_until_all_pes_finished();
    void set_total_pes(int n);

private:
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> event_queue;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    uint64_t current_time = 0;
    bool running_ = true;

    ClockMode mode_;
    std::unordered_map<int, EventHandler> handlers_;

    // Gestión de finalización de PE
    int pes_finished = 0;
    int total_pes = 0;
    std::mutex finish_mutex_;
    std::condition_variable finish_cv_;
};
