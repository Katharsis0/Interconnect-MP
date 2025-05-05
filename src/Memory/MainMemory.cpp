//
// Created by katharsis on 5/2/25.
//
#include "Memory/MainMemory.h"
#include <stdexcept>

#define NOT_OK 0x0
#define OK 0x1



std::vector<uint8_t> MainMemory::read(uint32_t addr, size_t size) {
    if (addr + size > MEM_SIZE * WORD_SIZE) {
        throw std::out_of_range("Memory read out of bounds");
    }

    std::vector<uint8_t> result;
    result.reserve(size);

    for (size_t i = 0; i < size; i++) {
        uint32_t word_addr = (addr + i) / WORD_SIZE;
        uint8_t byte_offset = (addr + i) % WORD_SIZE;
        uint32_t word = memory[word_addr];
        uint8_t byte = (word >> (byte_offset * 8)) & 0xFF;
        result.push_back(byte);
    }

    return result;
}

uint8_t MainMemory::write(uint32_t addr, const std::vector<uint8_t>& data) {
    if (addr + data.size() > MEM_SIZE * WORD_SIZE) {
        return NOT_OK;
    }

    for (size_t i = 0; i < data.size(); i++) {
        uint32_t word_addr = (addr + i) / WORD_SIZE;
        uint8_t byte_offset = (addr + i) % WORD_SIZE;
        uint32_t mask = ~(0xFF << (byte_offset * 8));
        memory[word_addr] = (memory[word_addr] & mask) | (data[i] << (byte_offset * 8));
    }

    return OK;
}