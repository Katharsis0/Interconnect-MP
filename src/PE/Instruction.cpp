#include "../../include/PE/Instruction.h"

Instruction::Instruction(InstructionType type, uint32_t address)
    : type(type), address(address) {}

Instruction::Instruction(InstructionType type, uint32_t address,
                         uint16_t size, uint8_t lines,
                         uint32_t start, std::vector<uint8_t> data)
    : type(type), address(address),
      size(size), num_of_cache_lines(lines),
      start_cache_line(start), data(std::move(data)) {}

uint32_t Instruction::getAddress() const { return address; }
InstructionType Instruction::getType() const { return type; }

uint16_t Instruction::getSize() const { return size; }
uint8_t Instruction::getNumLines() const { return num_of_cache_lines; }
uint32_t Instruction::getStartLine() const { return start_cache_line; }
const std::vector<uint8_t>& Instruction::getData() const { return data; }
