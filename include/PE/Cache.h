// Created by katharsis on 5/5/25.

#ifndef CACHE_H
#define CACHE_H

#include <array>
#include <cstdint>
#include <mutex>

#include "Interconnect/Interconnect.h"
#include "Messages/Messages.h"

constexpr uint16_t CACHE_LINE_SIZE = 16;      // 16 bytes per cache line
constexpr uint16_t CACHE_BLOCK_COUNT = 128;

class PE;
class Interconnect;

class Cache {
public:

    struct CacheLine {
        uint32_t tag;
        std::array<uint8_t, CACHE_LINE_SIZE> data {}; // Datos en la línea de caché
        bool valid;

        CacheLine() : tag(0), valid(false) {
            data.fill(0);
        }
    };

    // Constructor y destructor
    explicit Cache();
    ~Cache();


    // Setters
    void setOwnerPE(PE* pe);
    void setInterconnect(Interconnect* interconnect);

    // Operaciones de caché
    bool read(uint32_t address, uint8_t* data, uint16_t size);
    bool write(uint32_t address, const uint8_t* data, uint16_t size);
    bool invalidate(uint32_t address);
    void receiveMessage(const Message& msg);


    // Getters y utilidades
    uint32_t getTag(uint32_t address) const;
    uint32_t getIndex(uint32_t address) const;
    uint32_t getOffset(uint32_t address) const;
    PE* getPEOwner() const;

    // Estadísticas
    void printCacheContents() const;
    void printCacheStats() const;

    // Auxiliares
    uint32_t addressToBlockIndex(uint32_t address) const;
    uint32_t alignAddress(uint32_t address) const;

private:
    PE* owner_pe_ = nullptr;
    Interconnect* interconnect_ = nullptr;
    std::array<CacheLine, CACHE_BLOCK_COUNT> cache_lines_;
    std::mutex cache_mutex_; // Protección para concurrencia

    // Mensaje temporal (si se requiere)
    Message msg;

    // Estadísticas
    uint64_t reads_;
    uint64_t writes_;
    uint64_t invalidations_;
};

#endif // CACHE_H
