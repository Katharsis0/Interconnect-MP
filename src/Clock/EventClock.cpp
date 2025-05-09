#include "../../include/Clock/EventClock.h"
#include "../../include/Global/Global.h"
#include "../../include/Interconnect/Interconnect.h"
#include <iostream>

EventClock::EventClock(ClockMode mode)
    : mode_(mode), interconnect_(nullptr) {}

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
    // Esperar a que el hilo de input esté listo si estamos en modo Stepping
    if (mode_ == ClockMode::Stepping) {
        std::unique_lock<std::mutex> input_lock(step_mutex_);
        input_ready_cv_.wait(input_lock, [this]() { return input_thread_ready_; });
    }

    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !event_queue.empty() || !running_; });

        Event e = event_queue.top();
        event_queue.pop();
        current_time = e.timestamp;

        lock.unlock();

        {
            std::cout << "[T=" << std::dec << current_time << "] Evento: "
                      << e.action << " en PE " << e.pe_id << "\n";
        }

        // Ejecutar evento
        if (e.action == "interconnect_process" && interconnect_) {
            interconnect_->process_next();
        } else if (handlers_.count(e.pe_id)) {
            handlers_[e.pe_id](e);
        }

        // Esperar input si estamos en modo Stepping
        if (mode_ == ClockMode::Stepping) {
            std::cout << "[EventClock] Esperando [Enter] para siguiente evento...\n";
            std::cout << " Presiona [Enter] para continuar o 'q' para salir: ";
            std::cout.flush();

            std::unique_lock<std::mutex> step_lock(step_mutex_);
            step_cv_.wait(step_lock, [this]() { return step_ready_; });
            step_ready_ = false;
        }
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

ClockMode EventClock::get_mode() {
    return mode_;
}
