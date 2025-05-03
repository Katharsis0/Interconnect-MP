//
// Created by katharsis on 4/28/25.
//

#ifndef INSTRUCTIONMEMORY_H
#define INSTRUCTIONMEMORY_H
#pragma once

#include <vector>
#include "../Messages/Messages.h"
#include "../PE/Instruction.h"

class InstructionMemory {
public:
    void load(const std::vector<Instruction>& instructions);
    bool hasNext() const;
    Instruction getNext();
    void reset();

private:
    std::vector<Instruction> instructions_;
    size_t pc_ = 0; // Contador de programa
};
#endif //INSTRUCTIONMEMORY_H
