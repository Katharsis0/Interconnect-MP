#ifndef EVENTCLOCK_H
#define EVENTCLOCK_H

#include "Event.h"
#include "../Interconnect/Interconnect.h"
#include <queue>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

enum class ClockMode {
    Running,
    Stepping
};

class EventClock {
public:
    using EventHandler = std::function<void(const Event&)>;

    explicit EventClock(ClockMode mode);

    void set_interconnect(Interconnect* interconnect);
    void add_event(const Event& e);
    void run();
    void stop();
    uint64_t now() const;
    void register_pe(int pe_id, EventHandler handler);
    void set_total_pes(int n);
    void notify_pe_finished(int pe_id);
    void wait_until_all_pes_finished();

    ClockMode get_mode();

    // Stepping sync
    std::mutex step_mutex_;
    std::condition_variable step_cv_;
    bool step_ready_ = false;

    // Input thread coordination
    std::condition_variable input_ready_cv_;
    bool input_thread_ready_ = false;

    std::mutex input_ready_mutex_;
    bool input_ready_ = false;

private:
    ClockMode mode_;
    Interconnect* interconnect_;
    uint64_t current_time = 0;

    std::string scheme = "fifont"; // qos


    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> event_queue;
    std::unordered_map<int, EventHandler> handlers_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_ = true;

    // PE finish tracking
    std::mutex finish_mutex_;
    std::condition_variable finish_cv_;
    int total_pes = 0;
    int pes_finished = 0;


};

#endif // EVENTCLOCK_H
