#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

enum class InstructionType {
    READ,
    WRITE
};

class Instruction {
public:
    Instruction(InstructionType type, uint32_t address);

    bool isRead() const;
    bool isWrite() const;
    uint32_t getAddress() const;
    InstructionType getType() const;

private:
    InstructionType type_;
    uint32_t address_;
};

#endif // INSTRUCTION_H
