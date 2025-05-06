//
// Created by katharsis on 4/28/25.
//

#ifndef CACHE_H
#define CACHE_H


#pragma once

#include <cstdint>
#include <array>
#include <optional>

#include "Messages/Messages.h"

struct CacheLine {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    std::array<uint8_t, 16> data; // 16 bytes por línea de caché
};

class Cache {
public:

    struct CacheLine {
        uint32_t tag;
        std::array<uint8_t, CACHE_LINE_SIZE> data{}; // 16 bytes por línea de caché
        //Constructor de la linea
        CacheLine() : tag(0) {
            data.fill(0);
        }
    };

    explicit Cache(PE* owner_pe);

    PE* getPE() const;


    std::optional<std::array<uint8_t, 16>> read(uint32_t address); //Read lee desde memoria y almacena en cache
    bool write(uint32_t address, const std::array<uint8_t, 16>& data); //Write escribe desde cache hacia memoria

    //Recibir mensajes del PE
    void receiveMessagePE(const Message& msg);

    //Enviar mensaje al PE
    void sendMessagePE(const Message& msg);



private:
    std::array<CacheLine, NUM_LINES> lines_;

    uint32_t getTag(uint32_t address) const;
    uint32_t getIndex(uint32_t address) const;

};


#endif //CACHE_H
