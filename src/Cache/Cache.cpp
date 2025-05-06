//
// Created by katharsis on 5/5/25.
//

#include "Cache/Cache.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <mutex>

// Constructor
Cache::Cache(uint8_t pe_id)
    : pe_id_(pe_id), reads_(0), writes_(0), invalidations_(0) {
    // Initialize all cache lines
    for (auto& line : cache_lines_) {
        line.valid = false;
        line.state = MESIState::Invalid;
        line.tag = 0;
        line.data.fill(0);
    }
}

// Destructor
Cache::~Cache() = default;

// Read operation - return true if successful
bool Cache::read(uint32_t address, uint8_t* data, uint16_t size) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t offset = getOffset(address);
    uint32_t tag = getTag(address);

    // Check if the block is in cache
    if (cache_lines_[blockIndex].valid && cache_lines_[blockIndex].tag == tag) {
        //Cache hit
        if (cache_lines_[blockIndex].state == MESIState::Invalid) {
            return false; // Can't read invalid line
        }

        // Copy data from cache line to output buffer
        uint16_t bytesToCopy = std::min(size, static_cast<uint16_t>(CACHE_LINE_SIZE - offset));
        std::copy(
            cache_lines_[blockIndex].data.begin() + offset,
            cache_lines_[blockIndex].data.begin() + offset + bytesToCopy,
            data
        );

        reads_++;
        return true;
    }
    return false;
}

// Write operation: true if successful
bool Cache::write(uint32_t address, const uint8_t* data, uint16_t size) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t offset = getOffset(address);
    uint32_t tag = getTag(address);

    // Check cache state
    if (cache_lines_[blockIndex].valid && cache_lines_[blockIndex].tag == tag) {
        // Cache hit
        if (cache_lines_[blockIndex].state != MESIState::Modified &&
            cache_lines_[blockIndex].state != MESIState::Exclusive) {
            // Can only write if in Modified or Exclusive state
            return false;
        }

        // Write data to cache line
        uint16_t bytesToCopy = std::min(size, static_cast<uint16_t>(CACHE_LINE_SIZE - offset));
        std::copy(
            data,
            data + bytesToCopy,
            cache_lines_[blockIndex].data.begin() + offset
        );

        // Update state to Modified
        cache_lines_[blockIndex].state = MESIState::Modified;
        writes_++;
        return true;
    }
    return false;
}

//Invalidate a specific address
bool Cache::invalidate(uint32_t address) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t tag = getTag(address);

    if (cache_lines_[blockIndex].valid && cache_lines_[blockIndex].tag == tag) {
        cache_lines_[blockIndex].state = MESIState::Invalid;
        invalidations_++;
        return true;
    }

    return false; //Line not found in cache
}

//Invalidate specific cache line by index
bool Cache::invalidateLine(uint32_t cacheLineIndex) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    if (cacheLineIndex < CACHE_BLOCK_COUNT && cache_lines_[cacheLineIndex].valid) {
        cache_lines_[cacheLineIndex].state = MESIState::Invalid;
        invalidations_++;
        return true;
    }

    return false;
}

// Update MESI state for a cache line containing the given address
bool Cache::updateState(uint32_t address, MESIState newState) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t tag = getTag(address);

    if (cache_lines_[blockIndex].valid && cache_lines_[blockIndex].tag == tag) {
        cache_lines_[blockIndex].state = newState;
        return true;
    }

    return false;
}

//Get MESI state for a cache line containing the given address
MESIState Cache::getState(uint32_t address) {
    std::lock_guard<std::mutex> lock(cache_mutex_);

    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t tag = getTag(address);

    if (cache_lines_[blockIndex].valid && cache_lines_[blockIndex].tag == tag) {
        return cache_lines_[blockIndex].state;
    }

    return MESIState::Invalid;
}

// Get tag from memory address
uint32_t Cache::getTag(uint32_t address) const {
    // Tag = address / (cache_line_size * cache_block_count)
    return address / (CACHE_LINE_SIZE * CACHE_BLOCK_COUNT);
}

// Get index from memory address
uint32_t Cache::getIndex(uint32_t address) const {
    // Index = (address / cache_line_size) % cache_block_count
    return (address / CACHE_LINE_SIZE) % CACHE_BLOCK_COUNT;
}

// Get offset from memory address
uint32_t Cache::getOffset(uint32_t address) const {
    // Offset = address % cache_line_size
    return address % CACHE_LINE_SIZE;
}

// Check if address is in cache (hit)
bool Cache::isHit(uint32_t address) const {
    uint32_t blockIndex = addressToBlockIndex(address);
    uint32_t tag = getTag(address);

    return cache_lines_[blockIndex].valid &&
           cache_lines_[blockIndex].tag == tag &&
           cache_lines_[blockIndex].state != MESIState::Invalid;
}

// Get PE ID
uint8_t Cache::getPeId() const {
    return pe_id_;
}

// Convert address to cache block index
uint32_t Cache::addressToBlockIndex(uint32_t address) const {
    return getIndex(address);
}

// Align address to cache line boundary
uint32_t Cache::alignAddress(uint32_t address) const {
    return address & ~(CACHE_LINE_SIZE - 1);
}

// Print cache contents for debugging
void Cache::printCacheContents() const {
    std::cout << "Cache Contents for PE " << static_cast<int>(pe_id_) << ":" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "| Index | Tag      | State    | Valid   |" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    for (size_t i = 0; i < CACHE_BLOCK_COUNT; ++i) {
        if (cache_lines_[i].valid) {
            std::cout << "| " << std::setw(5) << i
                      << " | " << std::setw(8) << std::hex << cache_lines_[i].tag
                      << " | ";

            // Convert MESI state to string
            std::string state;
            switch (cache_lines_[i].state) {
                case MESIState::Modified: state = "Modified"; break;
                case MESIState::Exclusive: state = "Exclusive"; break;
                case MESIState::Shared: state = "Shared"; break;
                case MESIState::Invalid: state = "Invalid"; break;
                default: state = "Unknown";
            }

            std::cout << std::setw(8) << state
                      << " | " << (cache_lines_[i].valid ? "True" : "False")
                      << " |" << std::endl;
        }
    }

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << std::dec; // Reset to decimal output
}

// Print cache statistics
void Cache::printCacheStats() const {
    std::cout << "Cache Statistics for PE " << static_cast<int>(pe_id_) << ":" << std::endl;
    std::cout << "  Total Reads: " << reads_ << std::endl;
    std::cout << "  Total Writes: " << writes_ << std::endl;
    std::cout << "  Total Invalidations: " << invalidations_ << std::endl;
}