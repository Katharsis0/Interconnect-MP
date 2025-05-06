//
// Created by katharsis on 4/28/25.
//

#ifndef CACHE_H
#define CACHE_H



#pragma once

#include <cstdint>
#include <array>
#include <optional>

struct CacheLine {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    std::array<uint8_t, 16> data; // 16 bytes por línea de caché
};

class Cache {
public:
    static constexpr size_t NUM_LINES = 128; // 128 bloques como especificado

    Cache();

    std::optional<std::array<uint8_t, 16>> read(uint32_t address);
    bool write(uint32_t address, const std::array<uint8_t, 16>& data);
    void invalidate(uint32_t address);

private:
    std::array<CacheLine, NUM_LINES> lines_;

    uint32_t getTag(uint32_t address) const;
    uint32_t getIndex(uint32_t address) const;
};


#endif //CACHE_H
