#include "../../include/Clock/Scheduler.h"

Scheduler::Scheduler(EventClock& clock) : clock(clock) {}

void Scheduler::add_pe(PE* pe) {
    pes.push_back(pe);
}

void Scheduler::dispatch() {
    for (auto pe : pes) {
        pe->getStatistics();
    }
}
