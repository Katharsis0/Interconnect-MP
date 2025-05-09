#include "../include/Clock/EventClock.h"
#include "../include/PE/PE.h"
#include "../include/PE/Instruction.h"
#include "../include/Interconnect/Interconnect.h"
#include "../include/Global/Global.h"

#include <vector>
#include <memory>
#include <thread>
#include <iostream>

int main() {
    EventClock clock;
    //Interconnect interconnect(&clock);
    Interconnect interconnect;
    // Tell EventClock who the Interconnect is
    clock.set_interconnect(&interconnect);


    constexpr int NUM_PES = 8;
    clock.set_total_pes(NUM_PES);
    std::vector<Instruction> program = {
        Instruction(InstructionType::READ, 0x00001000, 4)
        //Instruction(InstructionType::WRITE, 0x2000, 0, 2, 0x2000, {0xDE, 0xAD, 0xBE, 0xEF}),
        //Instruction(InstructionType::INVALIDATE, 0x3000)
    };

    std::vector<Instruction> program1 = {
        //Instruction(InstructionType::READ,  0x1000),
        Instruction(InstructionType::WRITE, 0x1100),
        //Instruction(InstructionType::READ,  0x1200)
    };

    std::vector<Instruction> program2 = {
        Instruction(InstructionType::WRITE, 0x2000),
        //Instruction(InstructionType::READ,  0x2100)
    };

    std::vector<Instruction> program3 = {
        //Instruction(InstructionType::READ,  0x3000),
        Instruction(InstructionType::WRITE, 0x3100),
        //Instruction(InstructionType::READ,  0x3200),
        Instruction(InstructionType::WRITE, 0x3300)
    };

    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, 1, clock);

        // Assign programs to different PEs
        if (i < 2) {
            pe->loadInstructions(program);
        } else if (i < 5) {
            pe->loadInstructions(program);
        } else {
            pe->loadInstructions(program);
        }

        // Register PE in EventClock (for instruction_done, interconnect_process...)
        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->onEvent(e); // if PE i receives an event, call PE's onEvent()
        });

        // Register PE in Interconnect (for forwarding messages)
        interconnect.register_cache(i, &pe->getCache());

        // Save the PE into the local pes vector so main can start, stop, check statistics...
        pes.push_back(std::move(pe));
    }

    // Start all PEs
    for (auto& pe : pes) {
        pe->start();
    }

    // Start EventClock in a thread
    std::thread clock_thread(&EventClock::run, &clock);

    // Wait for all PEs to finish
    clock.wait_until_all_pes_finished();

    clock.stop();
    clock_thread.join();

    // Stop PEs
    for (auto& pe : pes) {
        pe->stop();
    }

    // Show statistics
    for (size_t i = 0; i < pes.size(); ++i) {
        auto stats = pes[i]->getStatistics();
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << i << ": " << stats.instructionsExecuted << " instrucciones ejecutadas\n";
    }

    return 0;
}
