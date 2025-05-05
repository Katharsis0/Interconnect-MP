#pragma once

#include <vector>
#include <cstdint>

class Cache {
private:
    static constexpr size_t CACHE_SIZE = 128;
    static constexpr size_t BLOCK_SIZE = 16;

    enum class CacheState {
        MODIFIED,   //Modified
        EXCLUSIVE,  //Exclusive clean
        SHARED,     //Shared clean
        INVALID     //Invalid
    };

    struct CacheLine {
        CacheState state;
        uint32_t tag;
        std::vector<uint8_t> data;

        CacheLine() : state(CacheState::INVALID), tag(0), data(BLOCK_SIZE, 0) {}
    };

    std::vector<CacheLine> lines;
    uint8_t pe_id;
public:
    explicit Cache(uint8_t id) : pe_id(id), lines(CACHE_SIZE) {}
    
    //Check if address is in cache
    bool contains(uint32_t addr) const;
    
    //Read data from cache
    std::vector<uint8_t> read(uint32_t addr, size_t size);
    
    //Write data to cache
    void write(uint32_t addr, const std::vector<uint8_t>& data);
    
    //Invalidate a cache line
    void invalidate(uint8_t line);
    
    //Get cache line and tag from address
    uint8_t getLineFromAddr(uint32_t addr) const;
    uint8_t getTagFromAddr(uint32_t addr) const;
};