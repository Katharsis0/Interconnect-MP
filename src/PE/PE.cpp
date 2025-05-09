//
// Created by katharsis on 4/28/25.
//

#include "../../include/PE/PE.h"

#include <iostream>

PE::PE(uint8_t id, uint8_t qos, EventClock& clock)
    : id_(id), qos_(qos), running_(false), clock_(clock), cache_(this){
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

//TODO: receiveMessageFromCache:
//BROADCAST_INVALIDATE -> El caché invalida una linea de caché y responde una confirmación de INV_ACK
//WRITE_RESP [DEST, STATUS, QoS] -> Respuesta a WRITE_MEM -> 0x0:Ok ^ 0x1:NOT_OK
//READ_RESP [DEST, DATA, QoS] -> Datos de la Memoria principal correspondient es a READ_ME
void PE::receiveMessageFromCache(const Message &msg) {
    std::lock_guard<std::mutex> lock(messagesMutex_);
    incomingMessages_.push(msg);
    messagesCV_.notify_one();
}

void PE::sendMessageToCache(const Message& msg) {
   cache_.receiveMessage(msg);
}




void PE::run() {
    while (running_ && instructionMemory_.hasNext()) {
        Instruction instr = instructionMemory_.getNext();

        if (instr.getType()== InstructionType::READ) {
            ReadMemMessage read;

            sendMessageToCache(read);

        } else if (instr.getType()== InstructionType::WRITE) {
            WriteMemMessage write;

            sendMessageToCache(write);

        } else if (instr.getType() == InstructionType::INVALIDATE) {
            BroadcastInvalidateMessage inv;
            sendMessageToCache(inv);
        }

        stats_.instructionsExecuted++;

        Event doneEvent;

        // Current time + latency
        doneEvent.timestamp = clock_.now() + 10; // Schedule this event to happen 10 logical time units after now
        doneEvent.pe_id = id_;
        doneEvent.action = "instruction_done";

        clock_.add_event(doneEvent);

        std::unique_lock<std::mutex> lock(pe_mutex_);
        pe_cv_.wait(lock);
    }

    // Notify the PE with id_ finished
    clock_.notify_pe_finished(id_);
}


void PE::onEvent(const Event& event) {
    if (event.action == "instruction_done") {
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << static_cast<int>(id_) << " reanuda ejecución\n";
        std::unique_lock<std::mutex> lock(pe_mutex_);
        pe_cv_.notify_all();
    }
}



PE::Statistics PE::getStatistics() const {
    return stats_;
}

uint8_t PE::getPE_id() const {
    return id_;
}

Cache& PE::getCache() {
    return cache_;
}