//
// Created by katharsis on 5/5/25.
//

#ifndef CACHE_H
#define CACHE_H
#include <array>
#include <cstdint>
#include <mutex>

#include "MESI/MESIState.h"



constexpr uint16_t CACHE_LINE_SIZE = 16;      // 16 bytes per cache line
constexpr uint16_t CACHE_BLOCK_COUNT = 128;

class Cache {
public:

    struct CacheLine {
        uint32_t tag;
        std::array<uint8_t, CACHE_LINE_SIZE> data {}; //Datos en la linea de cache
        MESIState state;
        bool valid;

        //Constructor de la linea
        CacheLine() : tag(0), state(MESIState::Invalid), valid(false) {
            data.fill(0);
        }
    };

    explicit Cache(uint8_t pe_id);

    //Destructor
    ~Cache();

    // MESI Protocol
    bool updateState(uint32_t address, MESIState newState);
    MESIState getState(uint32_t address);

    //Operaciones en cache
    bool read(uint32_t address, uint8_t* data, uint16_t size);
    bool write(uint32_t address, const uint8_t* data, uint16_t size);
    bool invalidate(uint32_t address);
    bool invalidateLine(uint32_t cacheLineIndex);

    //Utils
    uint32_t getTag(uint32_t address) const;
    uint32_t getIndex(uint32_t address) const;
    uint32_t getOffset(uint32_t address) const;
    bool isHit(uint32_t address) const;
    uint8_t getPeId() const;


    //Stats
    uint64_t reads_;
    uint64_t writes_;
    uint64_t invalidations_;

    // Debug and statistics
    void printCacheContents() const;
    void printCacheStats() const;

    //Auxiliares
    uint32_t addressToBlockIndex(uint32_t address) const;
    uint32_t alignAddress(uint32_t address) const;

private:
    uint8_t pe_id_;                         //PE ID
    std::array<CacheLine, CACHE_BLOCK_COUNT> cache_lines_;  //Cache lines
    std::mutex cache_mutex_;                //Mutex for thread safety

};

#endif //CACHE_H

