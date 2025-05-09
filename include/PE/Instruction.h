#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <vector>

enum class InstructionType {
    READ,
    WRITE,
    INVALIDATE
};

class Instruction {
public:
    Instruction(InstructionType type, uint32_t address);

    Instruction(InstructionType type, uint32_t address,
                uint16_t size, uint8_t lines, uint32_t start, std::vector<uint8_t> data);


    InstructionType getType() const;
    uint32_t getAddress() const;
    uint16_t getSize() const;
    uint8_t getNumLines() const;
    uint32_t getStartLine() const;
    const std::vector<uint8_t>& getData() const;

private:
    InstructionType type;
    uint32_t address;

    uint16_t size;
    uint8_t num_of_cache_lines;
    uint32_t start_cache_line;
    std::vector<uint8_t> data;
};

#endif
