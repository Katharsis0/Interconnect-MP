#include "../include/Clock/EventClock.h"
#include "../include/PE/PE.h"
#include "../include/PE/Instruction.h"
#include "../include/Interconnect/Interconnect.h"
#include "../include/Global/Global.h"

#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <mutex>

int main() {
    std::cout << "=== Starting simulation with 1 PE ===" << std::endl;

    EventClock clock;
    Interconnect interconnect;
    clock.set_interconnect(&interconnect);

    constexpr int NUM_PES = 1;
    clock.set_total_pes(NUM_PES);

    std::vector<Instruction> program = {
        Instruction(InstructionType::READ, 0x00000008, 4)  // leer 11223344
    };

    auto pe = std::make_unique<PE>(0, 1, clock);
    pe->loadInstructions(program);

    clock.register_pe(0, [pe_ptr = pe.get()](const Event& e) {
        pe_ptr->onEvent(e);
    });

    interconnect.register_cache(0, &pe->getCache());

    pe->start();

    std::thread clock_thread(&EventClock::run, &clock);
    clock.wait_until_all_pes_finished();

    clock.stop();
    clock_thread.join();

    pe->stop();

    auto stats = pe->getStatistics();
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::cout << "PE 0: " << stats.instructionsExecuted << " instrucciones ejecutadas\n";

    return 0;
}
