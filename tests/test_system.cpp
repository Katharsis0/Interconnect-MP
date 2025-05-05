//
// Created by katharsis on 5/5/25.
//
// test_system.cpp
#include "Interconnect/Interconnect.h"
#include "Memory/MainMemory.h"
#include "PE/PE.h"
#include <vector>

void test_coherence() {
    MainMemory mem;
    Interconnect interconnect(&mem, ArbitrationPolicy::FIFO);

    // Create 2 PEs with test instructions
    std::vector<std::string> pe1_instructions = {
        "WRITE_MEM 0x1000 1 0 0xFF",  // Write to address 0x1000
        "READ_MEM 0x1000 16 0xFF"     // Read back
    };

    std::vector<std::string> pe2_instructions = {
        "READ_MEM 0x1000 16 0x80",    // Read same address
        "WRITE_MEM 0x1000 1 0 0x80"   // Write to same address
    };

    PE pe1(0, pe1_instructions, &mem);
    PE pe2(1, pe2_instructions, &mem);

    interconnect.registerPE(&pe1);
    interconnect.registerPE(&pe2);

    // Set interconnect in each PE
    pe1.setInterconnect(&interconnect);
    pe2.setInterconnect(&interconnect);

    // Run simulation
    for (int i = 0; i < 100; i++) {
        pe1.nextInstruction();
        pe2.nextInstruction();
        interconnect.cycle();
    }

    interconnect.printStatistics();
}

int main() {
    test_coherence();
    return 0;
}