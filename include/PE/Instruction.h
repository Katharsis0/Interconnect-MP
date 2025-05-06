#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <vector>

enum class InstructionType {
    WRITE_MEM,
    READ_MEM,
    BROADCAST_INVALIDATE
};

class Instruction {
public:
    Instruction(InstructionType type, uint32_t address, uint16_t size = 0, uint8_t num_of_cache_lines = 0, uint32_t start_cache_line = 0);

    // Instruction queries
    bool isReadMem() const;
    bool isWriteMem() const;
    bool isBroadcastInvalidate() const;

    // Getters
    InstructionType getType() const;
    uint32_t getAddress() const;
    uint16_t getSize() const;
    uint8_t getNumOfCacheLines() const;
    uint32_t getStartCacheLine() const;

private:
    InstructionType type_;
    uint32_t address_;
    uint16_t size_;
    uint8_t num_of_cache_lines_;
    uint32_t start_cache_line_;
};

#endif // INSTRUCTION_H
