#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#pragma once
#include "../Messages/Messages.h"
#include <cstdint>
#include <iostream>

class Interconnect {
public:
    Interconnect() = default;

    void sendMessage(uint8_t source_id, uint8_t dest_id, const Message& message) {
        std::cout << "Interconnect: mensaje de PE " << int(source_id)
                  << " a PE " << int(dest_id) << " → tipo: " << int(getMessageType(message)) << "\n";

        std::cout << "Contenido: " << messageToString(message) << "\n";
    }
};

#endif //INTERCONNECT_H