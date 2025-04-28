//
// Created by katharsis on 4/28/25.
//

#ifndef PE_H
#define PE_H



#pragma once

#include <cstdint>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Messages/Messages.h"
#include "PE/Cache.h"
#include "PE/InstructionMemory.h"

class Interconnect; // Declaración anticipada

class PE {
public:
    PE(uint8_t id, uint8_t qos, Interconnect* interconnect);
    ~PE();

    void start();
    void stop();
    void loadInstructions(const std::vector<Instruction>& instructions);

    // Métodos para recibir mensajes del Interconnect
    void receiveMessage(const Message& msg);

    // Estadísticas
    struct Statistics {
        uint64_t instructionsExecuted;
        uint64_t cyclesBusy;
        // ... otras métricas
    };

    Statistics getStatistics() const;

private:
    void run(); // Función principal del thread del PE

    uint8_t id_;
    uint8_t qos_;
    Interconnect* interconnect_;
    std::thread thread_;
    bool running_;

    InstructionMemory instructionMemory_;
    Cache cache_;

    // Cola de mensajes entrantes
    std::queue<Message> incomingMessages_;
    mutable std::mutex messagesMutex_;
    std::condition_variable messagesCV_;

    Statistics stats_;
};


#endif //PE_H
