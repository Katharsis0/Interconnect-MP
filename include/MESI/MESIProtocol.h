#ifndef MESI_PROTOCOL_H
#define MESI_PROTOCOL_H

#include <unordered_map>
#include <cstdint>
#include "MESIState.h"

class MESIProtocol {
public:
    MESIProtocol(uint8_t pe_id);

    MESIState getState(uint32_t address) const;

    MESIState handleRead(uint32_t address);
    MESIState handleWrite(uint32_t address);
    void invalidate(uint32_t address);

private:
    uint8_t pe_id_;
    //guarda un MESIState por cada dirección de memoria en uint32_t
    std::unordered_map<uint32_t, MESIState> lineStates_;
};

#endif // MESI_PROTOCOL_H
