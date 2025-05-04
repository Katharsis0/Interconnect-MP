//
// Created by katharsis on 4/28/25.
//

#include "../../include/PE/PE.h"

#include <iostream>

PE::PE(uint8_t id, uint8_t qos, Interconnect* interconnect, EventClock& clock)
    : id_(id), qos_(qos), interconnect_(interconnect), running_(false), clock_(clock),
    instructionMemory_(), cache_(), mesiProtocol_(id){
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
        uint32_t addr = instr.getAddress(); // asumimos que tiene esta propiedad

        if (instr.isRead()) {
            MESIState state = mesiProtocol_.handleRead(addr);
            std::lock_guard<std::mutex> cout_lock(cout_mutex);
            std::cout << "PE " << static_cast<int>(id_) << " leyó dirección "
                      << std::hex << addr << " en estado " << static_cast<int>(state) << "\n";
        } else if (instr.isWrite()) {
            MESIState state = mesiProtocol_.handleWrite(addr);
            std::lock_guard<std::mutex> cout_lock(cout_mutex);
            std::cout << "PE " << static_cast<int>(id_) << " escribió dirección "
                      << std::hex << addr << " nuevo estado " << static_cast<int>(state) << "\n";
        }

        stats_.instructionsExecuted++;

        Event doneEvent;
        doneEvent.timestamp = clock_.now() + 10;
        doneEvent.pe_id = id_;
        doneEvent.action = "instruction_done";

        clock_.add_event(doneEvent);

        std::unique_lock<std::mutex> lock(pe_mutex_);
        pe_cv_.wait(lock);
    }

    // Avisar que este PE terminó
    clock_.notify_pe_finished(id_);
}


void PE::onEvent(const Event& event) {
    if (event.action == "instruction_done") {
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << static_cast<int>(id_) << " reanudando ejecución\n";
        std::unique_lock<std::mutex> lock(pe_mutex_);
        pe_cv_.notify_all();
    }
}



PE::Statistics PE::getStatistics() const {
    return stats_;
}

