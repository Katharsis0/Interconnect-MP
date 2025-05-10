#include "../../include/PE/Instruction.h"

Instruction::Instruction(InstructionType type, uint32_t address)
    : type(type), address(address) {}

Instruction::Instruction(InstructionType type, uint32_t address, uint16_t size)
    : type(type), address(address), size(size) {}

Instruction::Instruction(InstructionType type, uint32_t address,
                         uint16_t size, uint8_t lines,
                         uint32_t start, uint32_t cache_line, std::vector<uint32_t> data)
    : type(type), address(address),
      size(size), num_of_cache_lines(lines),
      start_cache_line(start), cache_line(cache_line), data(std::move(data)) {}

Instruction::Instruction(InstructionType type, uint32_t address,
                         uint16_t num_of_cache_lines, uint32_t start_cache_line, int8_t qos)
    : type(type), address(address),
      num_of_cache_lines(num_of_cache_lines),
      start_cache_line(start_cache_line),
      size(0), cache_line(0), data() {}



uint32_t Instruction::getAddress() const { return address; }
InstructionType Instruction::getType() const { return type; }

uint16_t Instruction::getSize() const { return size; }
uint8_t Instruction::getNumLines() const { return num_of_cache_lines; }
uint32_t Instruction::getStartLine() const { return start_cache_line; }
uint32_t Instruction::getCacheLine() const { return cache_line; }
const std::vector<uint32_t>& Instruction::getData() const { return data; }
