//
// Created by katharsis on 5/5/25.
//

#include "PE/Cache.h"
#include "PE/PE.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <mutex>

// Constructor
Cache::Cache(PE* owner_pe)
    : owner_pe(owner_pe), interconnect_(nullptr), reads_(0), writes_(0), invalidations_(0) {
    // Initialize all cache lines
    for (auto &line: cache_lines_) {
        line.valid = false;
        line.tag = 0;
        line.data.fill(0);
    }
}

// Destructor
Cache::~Cache() = default;

// Read operation - return true if successful
bool Cache::read(uint32_t address, uint8_t* data, uint16_t size) {

    return false;
}

// Write operation: true if successful
bool Cache::write(uint32_t address, const uint8_t* data, uint16_t size) {

    return false;
}

//Invalidate a specific address
bool Cache::invalidate(uint32_t address) {
    return false; //Line not found in cache
}

void Cache::setInterconnect(Interconnect* ic) {
    interconnect_ = ic;
}

void Cache::receiveMessage(const Message& msg) {
    if (!owner_pe) {
        std::cerr << "[Cache] ERROR: owner_pe is null!\n";
        return;
    }

    // Debug output
    std::cout << "[Cache] Received message: " << messageToString(msg) << std::endl;

    MessageType msgType = getMessageType(msg);

    if (msgType == MessageType::READ_MEM ||
        msgType == MessageType::BROADCAST_INVALIDATE) {
        std::cout << "[Cache] Mensaje dirigido para el Interconnect" << std::endl;

        // Verify interconnect is initialized
        if (!interconnect_) {
            std::cerr << "[Cache] ERROR: interconnect_ is null! Make sure register_cache was called." << std::endl;
            return;
        }

        // Forward message to interconnect
        interconnect_->receiveMessage(msg);
        }
    else if (msgType == MessageType::WRITE_MEM) {

        // Construct message to forward to interconnect
        WriteMemMessage forward;
        forward.src = getMessageSource(msg);
        forward.addr = getMessageAddress(msg);
        forward.start_cache_line = getCacheLine(msg);
        forward.num_of_cache_lines = getNumCacheLines(msg);
        forward.data = getDataFromCacheLine(forward.start_cache_line, forward.num_of_cache_lines);
        std::cout << "[Cache] Forwarding WRITE_MEM to Interconnect:\n" << forward.toString() << std::endl;

        if (interconnect_) {
            interconnect_->receiveMessage(forward);
        } else {
            std::cerr << "[Cache] ERROR: interconnect_ is null!" << std::endl;
        }
    }
    else if (msgType == MessageType::READ_RESP ||
             msgType == MessageType::WRITE_RESP ||
             msgType == MessageType::INV_ACK ||
             msgType == MessageType::INV_COMPLETE) {
        std::cout << "[Cache] Mensaje dirigido para el OwnerPE" << std::endl;

        // Forward message to owner PE
        owner_pe->receiveMessageFromCache(msg);
        } else {
            std::cerr << "[Cache] ERROR: owner_pe is null when forwarding response!" << std::endl;
        }
}


std::vector<uint32_t> Cache::getDataFromCacheLine(uint32_t start_line, uint32_t num_lines) {

    std::vector<uint32_t> result;
    for (uint32_t i = 0; i < num_lines; ++i) {
        uint32_t index = start_line + i;
        if (index >= CACHE_BLOCK_COUNT) break;
        const auto& line = cache_lines_[index];
        result.insert(result.end(), line.data.begin(), line.data.end());
    }
    return result;
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



// Get PE ID
PE* Cache::getPEOwner() const {
    return this->owner_pe;
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

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << std::dec; // Reset to decimal output
}

