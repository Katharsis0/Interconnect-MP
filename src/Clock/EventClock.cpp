#include "../../include/Clock/EventClock.h"
#include "../../include/Global/Global.h"
#include <iostream>

EventClock::EventClock(ClockMode mode)
    : mode_(mode) {}

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
            std::cout << "[T=" << std::dec << current_time << "] Evento: " << e.action
                      << " en PE " << e.pe_id << "\n";
        }

        if (handlers_.count(e.pe_id)) {
            handlers_[e.pe_id](e);
        }

        if (mode_ == ClockMode::Stepping)
            break;
    }
}

bool EventClock::run_step() {
    std::unique_lock<std::mutex> lock(mutex_);

    if (event_queue.empty())
        return false;

    Event e = event_queue.top();
    event_queue.pop();
    current_time = e.timestamp;

    lock.unlock();

    {
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "[T=" << std::dec << current_time << "] Evento: " << e.action
                  << " en PE " << e.pe_id << "\n";
    }

    if (handlers_.count(e.pe_id)) {
        handlers_[e.pe_id](e);
    }

    return true;
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

void EventClock::set_total_pes(int n) {
    total_pes = n;
}

void EventClock::notify_pe_finished(int pe_id) {
    std::lock_guard<std::mutex> lock(finish_mutex_);
    pes_finished++;

    {
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << pe_id << " ha terminado. Total terminados: "
                  << pes_finished << "/" << total_pes << "\n";
    }

    finish_cv_.notify_all();
}

void EventClock::wait_until_all_pes_finished() {
    std::unique_lock<std::mutex> lock(finish_mutex_);
    finish_cv_.wait(lock, [this]() {
        return pes_finished >= total_pes;
    });
}
