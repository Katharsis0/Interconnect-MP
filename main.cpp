//
// Created by katharsis on 5/5/25.
//
// test_main.cpp
#include "../include/Cache/Cache.h"
#include "../include/MESI/MESIProtocol.h"
#include "../include/PE/PE.h"
#include "../include/Clock/EventClock.h"
#include "../include/Messages/Messages.h"

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

#include "Interconnect/Interconnect.h"

std::vector program = {
    Instruction(InstructionType::READ_MEM, 0x1000),
    Instruction(InstructionType::WRITE_MEM, 0x1000),
    Instruction(InstructionType::READ_MEM, 0x2000),
    Instruction(InstructionType::WRITE_MEM, 0x2000),
    Instruction(InstructionType::BROADCAST_INVALIDATE, 0x0),
};

// ---------------- PE Instruction Test ----------------
void Test_PE_Instruction_Parsing() {
    std::cout << "--- PE Instruction Test ---\n";

    EventClock clock;
    clock.set_total_pes(1);

    PE pe(0, 0x0, clock);



    pe.loadInstructions(program);
    pe.start();


    clock.register_pe(0, [&pe](const Event& e) {
        pe.onEvent(e);
    });

    // Start EventClock in a thread
    std::thread clock_thread(&EventClock::run, &clock);


    clock.wait_until_all_pes_finished();

    clock.stop();
    clock_thread.join();

    pe.stop();
    std::cout << "[Test] PE program executed.\n";
}




// ---------------- EventClock Test ----------------
void Test_EventClock() {
    std::cout << "--- EventClock Scheduling Test ---\n";

    EventClock clock;
    clock.set_total_pes(3);


    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < 3; ++i) {
        auto pe = std::make_unique<PE>(i, 1, clock);
        pe->loadInstructions(program);


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

        clock.add_event({.timestamp = 1, .pe_id = 0});
        clock.add_event({.timestamp = 2, .pe_id = 1});
        clock.add_event({.timestamp = 3, .pe_id = 2});


        std::cout << "[Test] EventClock simulation complete.\n";
    }


// ---------------- Main ----------------
int main() {
    std::cout << "============================\n";
    std::cout << "   Running Unit Tests       \n";
    std::cout << "============================\n\n";

    Test_PE_Instruction_Parsing();
    std::cout << "\n";


    Test_EventClock();
    std::cout << "\n";

    std::cout << "ALL TESTS COMPLETED\n";
    return 0;
}
