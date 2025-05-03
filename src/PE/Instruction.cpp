#include "../../include/PE/Instruction.h"

Instruction::Instruction(InstructionType type, uint32_t address)
    : type_(type), address_(address) {}

bool Instruction::isRead() const {
    return type_ == InstructionType::READ;
}

bool Instruction::isWrite() const {
    return type_ == InstructionType::WRITE;
}

uint32_t Instruction::getAddress() const {
    return address_;
}

InstructionType Instruction::getType() const {
    return type_;
}
