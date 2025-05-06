#include "../../include/PE/Instruction.h"

Instruction::Instruction(InstructionType type, uint32_t address, uint16_t size, uint8_t num_of_cache_lines, uint32_t start_cache_line)
    : type_(type), address_(address), size_(size), num_of_cache_lines_(num_of_cache_lines), start_cache_line_(start_cache_line) {}

bool Instruction::isReadMem() const {
    return type_ == InstructionType::READ_MEM;
}

bool Instruction::isWriteMem() const {
    return type_ == InstructionType::WRITE_MEM;
}

bool Instruction::isBroadcastInvalidate() const {
    return type_ == InstructionType::BROADCAST_INVALIDATE;
}

InstructionType Instruction::getType() const {
    return type_;
}

uint32_t Instruction::getAddress() const {
    return address_;
}

uint16_t Instruction::getSize() const {
    return size_;
}

uint8_t Instruction::getNumOfCacheLines() const {
    return num_of_cache_lines_;
}

uint32_t Instruction::getStartCacheLine() const {
    return start_cache_line_;
}
