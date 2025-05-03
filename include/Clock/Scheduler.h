#pragma once
#include "EventClock.h"
#include "../PE/PE.h"
#include <vector>

class Scheduler {
public:
    Scheduler(EventClock& clock);
    void add_pe(PE* pe);
    void dispatch();

private:
    EventClock& clock;
    std::vector<PE*> pes;
};
