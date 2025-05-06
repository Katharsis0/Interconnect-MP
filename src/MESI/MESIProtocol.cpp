#include "MESI/MESIProtocol.h"

MESIProtocol::MESIProtocol(uint8_t pe_id)
    : pe_id_(pe_id) {}

MESIState MESIProtocol::getState(uint32_t address) const {
    auto it = lineStates_.find(address);
    if (it != lineStates_.end()) return it->second;
    return MESIState::Invalid;
}

// Esto simula que si se lee una dirección inválida, se trae el dato desde memoria si se marca como exclusivo (porque ningún otro PE lo tiene en teoría).

MESIState MESIProtocol::handleRead(uint32_t address) {
    auto& state = lineStates_[address];
    switch (state) {
        case MESIState::Invalid:
            state = MESIState::Exclusive;  // Supone que nadie más tiene la línea
            break;
        case MESIState::Shared:
        case MESIState::Exclusive:
        case MESIState::Modified:
            // Se mantiene en el mismo estado para lectura
            break;
    }
    return state;
}

MESIState MESIProtocol::handleWrite(uint32_t address) {
    auto& state = lineStates_[address];
    switch (state) {
        case MESIState::Invalid:
        case MESIState::Shared:
        case MESIState::Exclusive:
            state = MESIState::Modified;
            break;
        case MESIState::Modified:
            // ya está en estado correcto
            break;
    }
    return state;
}

void MESIProtocol::invalidate(uint32_t address) {
    lineStates_[address] = MESIState::Invalid;
}
