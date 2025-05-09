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
    constexpr int NUM_PES = 8;  // 🔁 Change this value to test 1 or more PEs
    std::cout << "=== Starting simulation with " << NUM_PES << " PE(s) ===" << std::endl;

    EventClock clock;
    Interconnect interconnect;
    clock.set_interconnect(&interconnect);
    clock.set_total_pes(NUM_PES);

    std::vector<Instruction> program = {
        Instruction(InstructionType::READ, 0x00000008, 4)  // Each PE reads same address
    };

    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, 1, clock);
        pe->loadInstructions(program);

        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->onEvent(e);
        });

        interconnect.register_cache(i, &pe->getCache());
        pes.push_back(std::move(pe));
    }

    for (auto& pe : pes) pe->start();

    std::thread clock_thread(&EventClock::run, &clock);
    clock.wait_until_all_pes_finished();
    clock.stop();
    clock_thread.join();

    for (auto& pe : pes) pe->stop();

    for (size_t i = 0; i < pes.size(); ++i) {
        auto stats = pes[i]->getStatistics();
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << i << ": " << stats.instructionsExecuted << " instrucciones ejecutadas\n";
    }

    return 0;
}
