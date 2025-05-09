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
Cache::Cache()
    : owner_pe_(nullptr), interconnect_(nullptr),
      reads_(0), writes_(0), invalidations_(0) {
    for (auto& line : cache_lines_) {
        line.valid = false;
        line.tag = 0;
        line.data.fill(0);
    }
}

Cache::~Cache() = default;

void Cache::setOwnerPE(PE* pe) {
    owner_pe_ = pe;
    std::cout << "[Cache] owner_pe set to PE " << static_cast<int>(pe->getPE_id()) << "\n";
}

void Cache::setInterconnect(Interconnect* ic) {
    interconnect_ = ic;
    if (owner_pe_)
        std::cout << "[Cache] Interconnect pointer set for PE " << static_cast<int>(owner_pe_->getPE_id()) << "\n";
    else
        std::cout << "[Cache] Interconnect pointer set for PE ??? (owner_pe is null!)\n";
}

PE* Cache::getPEOwner() const {
    return owner_pe_;
}
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


void Cache::receiveMessage(const Message& msg) {

    if (getMessageType(msg) == MessageType::READ_MEM ||
        getMessageType(msg) == MessageType::WRITE_MEM ||
        getMessageType(msg) == MessageType::BROADCAST_INVALIDATE) {
        //Notify interconnect
        interconnect_->send(getPEOwner()->getPE_id(), msg);
    }
    else if (getMessageType(msg)== MessageType::READ_RESP ||
            getMessageType(msg) == MessageType::WRITE_RESP ||
            getMessageType(msg)== MessageType::INV_ACK ||
            getMessageType(msg) == MessageType::INV_COMPLETE) {
        //Notify owner
        owner_pe_->receiveMessageFromCache(msg);
    }
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

