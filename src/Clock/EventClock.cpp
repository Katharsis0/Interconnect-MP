#include "../../include/Clock/EventClock.h"
#include "../../include/Global/Global.h"

#include <iostream>

void EventClock::add_event(const Event& e) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        event_queue.push(e);
    }
    cv_.notify_one();
}

void EventClock::run() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]() { return !event_queue.empty() || !running_; });

        if (!running_) break;

        Event e = event_queue.top();
        event_queue.pop();

        current_time = e.timestamp;

        lock.unlock();

        {
            std::lock_guard<std::mutex> cout_lock(cout_mutex);
            std::cout << "[T=" << current_time << "] Evento: " << e.action
                      << " en PE " << e.pe_id << "\n";
        }

        // Cout mutex no longer blocked
        if (handlers_.count(e.pe_id)) {
            handlers_[e.pe_id](e);
        }
    }
}


void EventClock::stop() {
    running_ = false;
    cv_.notify_all();
}

uint64_t EventClock::now() const {
    return current_time;
}

void EventClock::register_pe(int pe_id, EventHandler handler) {
    handlers_[pe_id] = handler;
}
