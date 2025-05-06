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
    Interconnect interconnect;

    constexpr int NUM_PES = 8;
    clock.set_total_pes(NUM_PES);

    std::vector<Instruction> program1 = {
        Instruction(InstructionType::READ,  0x1000),
        Instruction(InstructionType::WRITE, 0x1100),
        Instruction(InstructionType::READ,  0x1200),
        Instruction(InstructionType::WRITE, 0x1300),
        Instruction(InstructionType::READ,  0x1400),
        Instruction(InstructionType::READ,  0x3600),
        Instruction(InstructionType::READ,  0x3700),
        Instruction(InstructionType::WRITE, 0x3800),
        Instruction(InstructionType::READ,  0x3900),
        Instruction(InstructionType::WRITE, 0x3A00),
        Instruction(InstructionType::READ,  0x3400),
        Instruction(InstructionType::WRITE, 0x3500)
    };

    std::vector<Instruction> program2 = {
        Instruction(InstructionType::WRITE, 0x2000),
        Instruction(InstructionType::READ,  0x2100),
        Instruction(InstructionType::WRITE, 0x2200),
        Instruction(InstructionType::READ,  0x2300),
        Instruction(InstructionType::READ,  0x2400),
        Instruction(InstructionType::WRITE, 0x2500),
        Instruction(InstructionType::READ,  0x2600)
    };

    std::vector<Instruction> program3 = {
        Instruction(InstructionType::READ,  0x3000),
        Instruction(InstructionType::WRITE, 0x3100),
        Instruction(InstructionType::READ,  0x3200),
        Instruction(InstructionType::WRITE, 0x3300)
    };


    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, 1, &interconnect, clock);

        // Asignar programas distintos a los PEs
        if (i < 2) {
            pe->loadInstructions(program1); // PE 0, PE 1
        } else if (i < 5) {
            pe->loadInstructions(program2); // PE 2, PE 3, PE 4
        } else {
            pe->loadInstructions(program3); // PE 5, PE 6, PE 7
        }

        //  When EventClock has event for PE i, call pe_ptr->onEvent(event)
        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->onEvent(e); // Allows EventClock to notify PE
        });

        pes.push_back(std::move(pe));
    }

    // Start all PEs
    for (auto& pe : pes) {
        pe->start();
    }

    // Start EventClock in a thread
    std::thread clock_thread(&EventClock::run, &clock);


    clock.wait_until_all_pes_finished();

    clock.stop();
    clock_thread.join();

    for (auto& pe : pes) {
        pe->stop();
    }

    for (size_t i = 0; i < pes.size(); ++i) {
        auto stats = pes[i]->getStatistics();
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << i << ": " << stats.instructionsExecuted << " instrucciones ejecutadas\n";
    }

    return 0;
}
