//
// Created by katharsis on 4/28/25.
//

#ifndef PE_H
#define PE_H



#pragma once
#include "../Clock/EventClock.h"
#include "../../include/Global/Global.h"

#include <cstdint>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "../Messages/Messages.h"
#include "../PE/Cache.h"
#include "../PE/InstructionMemory.h"


class Interconnect; // Declaración anticipada

class PE {
public:
    PE(uint8_t id, uint8_t qos, EventClock& clock);

    ~PE();

    void start();
    void stop();
    void loadInstructions(const std::vector<Instruction>& instructions);

    // Métodos para recibir mensajes del Interconnect
    void receiveMessageFromCache(const Message& msg);


    void onEvent(const Event& event);

    void sendMessageToCache(const Message& msg);


    // Estadísticas
    struct Statistics {
        uint64_t instructionsExecuted;
        uint64_t cyclesBusy;
    };

    Statistics getStatistics() const;

    uint8_t getPE_id() const;

    Cache& getCache();


private:
    void run(); // Función principal del thread del PE

    uint8_t id_;
    uint8_t qos_;
    std::thread thread_;
    bool running_;

    EventClock& clock_;
    std::mutex pe_mutex_;
    std::condition_variable pe_cv_;

    InstructionMemory instructionMemory_;
    Cache cache_;

    // Cola de mensajes entrantes
    std::queue<Message> incomingMessages_;
    mutable std::mutex messagesMutex_;
    std::condition_variable messagesCV_;

    Statistics stats_{};
};


#endif //PE_H
