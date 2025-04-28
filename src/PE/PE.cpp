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
    while (running_) {
        // Procesar mensajes entrantes
        Message msg;
        {
            std::unique_lock<std::mutex> lock(messagesMutex_);
            messagesCV_.wait(lock, [this]() {
                return !running_ || !incomingMessages_.empty();
            });

            if (!running_) break;

            msg = incomingMessages_.front();
            incomingMessages_.pop();
        }

        // Procesar el mensaje según su tipo
        // (Implementar lógica para cada tipo de mensaje)

        // Ejecutar siguiente instrucción
        if (instructionMemory_.hasNext()) {
            auto instruction = instructionMemory_.getNext();
            // Convertir instrucción en mensaje y enviar al Interconnect
            // interconnect_->sendMessage(message);
            stats_.instructionsExecuted++;
        }

        stats_.cyclesBusy++;
    }
}

PE::Statistics PE::getStatistics() const {
    return stats_;
}