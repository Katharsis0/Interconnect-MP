//
// Created by katharsis on 4/28/25.
//

#include "../../include/PE/PE.h"

#include <iostream>

PE::PE(uint8_t id, uint8_t qos, Interconnect* interconnect)
    : id_(id), qos_(qos), interconnect_(interconnect), running_(false) {
    // Inicializar estadísticas
    stats_ = Statistics{};
}

PE::~PE() {
    stop();
}

void PE::start() {
    if (!running_) {
        running_ = true;
        thread_ = std::thread(&PE::run, this);
    }
}

void PE::stop() {
    running_ = false;
    messagesCV_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void PE::loadInstructions(const std::vector<Instruction>& instructions) {
    instructionMemory_.load(instructions);
}

void PE::receiveMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(messagesMutex_);
    incomingMessages_.push(msg);
    messagesCV_.notify_one();
}

void PE::run() {
    while (running_ && instructionMemory_.hasNext()) {
        Instruction instr = instructionMemory_.getNext();
        uint32_t addr = instr.address; // asumimos que tiene esta propiedad

        if (instr.isRead()) {
            MESIState state = mesiProtocol_.handleRead(addr);
            std::cout << "PE " << static_cast<int>(id_) << " leyó dirección "
                      << std::hex << addr << " en estado " << static_cast<int>(state) << "\n";
        } else if (instr.isWrite()) {
            MESIState state = mesiProtocol_.handleWrite(addr);
            std::cout << "PE " << static_cast<int>(id_) << " escribió dirección "
                      << std::hex << addr << " nuevo estado " << static_cast<int>(state) << "\n";
        }

        // estadísticas y espera artificial DEBE CAMBIARSE EL SLEEPP, ESTE ESTÄ DE PRUEBA
        stats_.instructionsExecuted++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


PE::Statistics PE::getStatistics() const {
    return stats_;
}

PE::PE(uint8_t id, uint8_t qos, Interconnect* interconnect)
    : id_(id), qos_(qos), interconnect_(interconnect),
      instructionMemory_(), cache_(), mesiProtocol_(id),
      running_(false) {}
