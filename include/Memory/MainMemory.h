//
// Created by katharsis on 5/2/25.
//

#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#pragma once

#include <vector>
#include <cstdint>
#include "../Messages/Messages.h"

class MainMemory {
private:
    static constexpr size_t MEM_SIZE = 4096;    //4096 positions
    static constexpr size_t WORD_SIZE = 4;      //32-bit words

    std::vector<uint32_t> memory;

public:
    MainMemory() : memory(MEM_SIZE, 0) {}

    //Memory read
    std::vector<uint8_t> read(uint32_t addr, size_t size);

    //Memory write
    uint8_t write(uint32_t addr, const std::vector<uint8_t>& data);
};
#endif //MAINMEMORY_H
