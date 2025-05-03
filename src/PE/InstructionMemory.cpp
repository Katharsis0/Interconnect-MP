//
// Created by katharsis on 4/28/25.
//
#include "../../include/PE/InstructionMemory.h"

void InstructionMemory::load(const std::vector<Instruction>& instructions) {
    instructions_ = instructions;
    pc_ = 0;
}

bool InstructionMemory::hasNext() const {
    return pc_ < instructions_.size();
}

Instruction InstructionMemory::getNext() {
    if (hasNext()) {
        return instructions_[pc_++];
    }

    // Si no hay instrucciones, devolver dummy para evitar error
    return Instruction(InstructionType::READ, 0x0);
}

void InstructionMemory::reset() {
    pc_ = 0;
}
