//
// Created by katharsis on 4/28/25.
//

#include "../../include/PE/Cache.h"

Cache::Cache(): lines_() {
    // Constructor vacío
}

std::optional<std::array<uint8_t, 16>> Cache::read(uint32_t address) {
    // Implementación vacía para que compile
    return std::nullopt;
}

bool Cache::write(uint32_t address, const std::array<uint8_t, 16>& data) {
    // Implementación vacía para que compile
    return false;
}

void Cache::invalidate(uint32_t address) {
    // Implementación vacía para que compile
}

uint32_t Cache::getTag(uint32_t address) const {
    // Implementación vacía para que compile
    return 0;
}

uint32_t Cache::getIndex(uint32_t address) const {
    // Implementación vacía para que compile
    return 0;
}
