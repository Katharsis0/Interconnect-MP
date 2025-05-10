#include "../../include/Clock/EventClock.h"
#include "../../include/Global/Global.h"
#include "../../include/Interconnect/Interconnect.h"
#include <iostream>

// Constructor
EventClock::EventClock(ClockMode mode)
    : mode_(mode),
      interconnect_(nullptr),
      running_(true),
      current_time(0),
      pes_finished(0),
      total_pes(0)
{}

void EventClock::set_interconnect(Interconnect* interconnect) {
    interconnect_ = interconnect;
}

void EventClock::add_event(const Event& e) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        event_queue.push(e);
    }
    cv_.notify_all();
}

void EventClock::run() {
    // Stepping: wait for input thread ready
    if (mode_ == ClockMode::Stepping) {
        std::unique_lock<std::mutex> lk(step_mutex_);
        input_ready_cv_.wait(lk, [this]{ return input_thread_ready_; });
    }

    while (running_) {
        Event e;
        {
            std::unique_lock<std::mutex> lk(mutex_);
            cv_.wait(lk, [this]{ return !event_queue.empty() || !running_; });
            if (!running_) break;
            e = event_queue.top();
            event_queue.pop();
            current_time = e.timestamp;
        }

        if (mode_ == ClockMode::Stepping) {
            std::cout << "[T=" << current_time << "] Press [Enter] to continue or 'q' to quit: ";
            std::cout.flush();
            std::unique_lock<std::mutex> lk(step_mutex_);
            step_cv_.wait(lk, [this]{ return step_ready_; });
            step_ready_ = false;
        }
        if (e.action == "interconnect_process" && interconnect_) {
            interconnect_->process_next();
        } else if (handlers_.count(e.pe_id)) {
            handlers_[e.pe_id](e);
        }

    }

    // Shutdown: notify all PEs
    for (auto& kv : handlers_) {
        kv.second({ .timestamp = 0, .pe_id = kv.first, .action = "shutdown" });
    }
}

void EventClock::stop() {
    running_ = false;
    cv_.notify_all();
    step_cv_.notify_all();
}

uint64_t EventClock::now() const {
    return current_time;
}

void EventClock::register_pe(int pe_id, EventHandler handler) {
    handlers_[pe_id] = handler;
}

void EventClock::set_total_pes(int n) {
    total_pes = n;
}

void EventClock::notify_pe_finished(int pe_id) {
    std::lock_guard<std::mutex> lk(finish_mutex_);
    pes_finished++;
    if (pes_finished >= total_pes) {
        finish_cv_.notify_all();
    }
}

void EventClock::wait_until_all_pes_finished() {
    std::unique_lock<std::mutex> lk(finish_mutex_);
    finish_cv_.wait(lk, [this]{ return pes_finished >= total_pes; });
}

ClockMode EventClock::get_mode() {
    return mode_;
}