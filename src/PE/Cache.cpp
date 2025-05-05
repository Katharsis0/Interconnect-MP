//
// Created by katharsis on 4/28/25.
//

#include "../../include/PE/Cache.h"
#include <iostream>

#include "Messages/Messages.h"


std::vector<uint8_t> Cache::read(uint32_t addr, size_t size) {
    uint8_t line_idx = getLineFromAddr(addr);
    uint32_t tag = getTagFromAddr(addr);

    if (lines[line_idx].state == CacheState::INVALID ||
        lines[line_idx].tag != tag) {
        // Cache miss - need to fetch from memory
        Message msg(MessageType::READ_MEM, pe_id, 0);
        msg.addr = addr & ~0xF; // Align to cache line
        msg.size = BLOCK_SIZE;
        // Send to interconnect FALTA ESTO
        interconnect->sendMessage(msg);

        //Ahorita es cero
        return std::vector<uint8_t>(size, 0);
        }

    // Handle hit
    uint8_t offset = addr & 0xF;
    std::vector<uint8_t> result;
    size_t bytes_to_copy = std::min(size, BLOCK_SIZE - offset);
    result.insert(result.end(),
                 lines[line_idx].data.begin() + offset,
                 lines[line_idx].data.begin() + offset + bytes_to_copy);
    return result;
}

void Cache::write(uint32_t addr, const std::vector<uint8_t>& data) {
    uint8_t line_idx = getLineFromAddr(addr);
    uint32_t tag = getTagFromAddr(addr);

    if (lines[line_idx].state == CacheState::INVALID ||
        lines[line_idx].tag != tag) {
        // Need to get exclusive ownership
        Message msg(MessageType::BROADCAST_INVALIDATE, pe_id, 0);
        msg.cache_line = line_idx;
        interconnect->sendMessage(msg);

        // Wait for INV_COMPLETE
        }

    // Update cache line
    uint8_t offset = addr & 0xF;
    size_t bytes_to_copy = std::min(data.size(), BLOCK_SIZE - offset);
    std::copy(data.begin(), data.begin() + bytes_to_copy,
              lines[line_idx].data.begin() + offset);

    lines[line_idx].state = CacheState::MODIFIED;
    lines[line_idx].tag = tag;
}

void Cache::invalidate(uint8_t line) {
    if (line < CACHE_SIZE) {
        if (lines[line].dirty) {
            std::cout << "PE " << (int)pe_id << ": Removing dirty line "
                      << (int)line << std::endl;
        }
        lines[line].valid = false;
    }
}

uint8_t Cache::getLineFromAddr(uint32_t addr) const {
    return (addr >> 4) & 0x7F; //128 lines = 7 bits
}
uint8_t Cache::getTagFromAddr(uint32_t addr) const {
    return (addr >> 4) & 0x7F;//128 lines = 7 bits
}