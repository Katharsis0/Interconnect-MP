//
// Created by katharsis on 4/28/25.
//

// PE.cpp
#include "../../include/PE/PE.h"
#include "../../include/Clock/EventClock.h"
#include "../../include/Messages/Messages.h"
#include <iostream>
#include <algorithm>

PE::PE(uint8_t id, uint8_t qos, EventClock& clock)
    : id_(id), qos_(qos), running_(false), clock_(clock), cache_(this) {
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
    // wake run() if it's waiting
    inbox_cv_.notify_all();
    if (thread_.joinable()) thread_.join();
}

void PE::loadInstructions(const std::vector<Instruction>& instructions) {
    instructionMemory_.load(instructions);
}

void PE::enqueueEvent(const Event& e) {
    {
        std::lock_guard<std::mutex> lk(inbox_mtx_);
        inbox_.push(e);
    }
    inbox_cv_.notify_one();
}





//TODO: receiveMessageFromCache:
//BROADCAST_INVALIDATE -> El caché invalida una linea de caché y responde una confirmación de INV_ACK
//WRITE_RESP [DEST, STATUS, QoS] -> Respuesta a WRITE_MEM -> 0x0:Ok ^ 0x1:NOT_OK
//READ_RESP [DEST, DATA, QoS] -> Datos de la Memoria principal correspondient es a READ_ME
void PE::receiveMessageFromCache(const Message &msg) {
    if (getMessageType(msg) == MessageType::READ_RESP) {
        const auto& resp = std::get<ReadRespMessage>(msg);
        const std::vector<uint8_t>& data = resp.data;

        std::cout << "[PE " << static_cast<int>(id_) << "] received READ_RESP with "
                  << data.size() << " bytes.\n";

        //Get the cache
        Cache& cache = getCache();

        //Find next available line (debe ser invalid)
        for (int i = 0; i < CACHE_BLOCK_COUNT; ++i) {
            if (!cache.cache_lines_[i].valid) {
                cache.cache_lines_[i].valid = true;

                //Warning
                if (data.size() > cache.cache_lines_[i].data.size()) {
                    std::cerr << "[PE " << static_cast<int>(id_) << "] WARNING: READ_RESP size ("
                              << data.size() << ") > cache line size ("
                              << cache.cache_lines_[i].data.size() << ")" << std::endl;
                }

                size_t to_copy = std::min(data.size(), cache.cache_lines_[i].data.size());
                std::copy(data.begin(), data.begin() + to_copy, cache.cache_lines_[i].data.begin());


                std::cout << "[PE " << static_cast<int>(id_) << "] wrote "
                          << to_copy << " bytes to cache line " << i << std::endl;
                break;
            }

        }
    }

        Event doneEvent;

        int64_t net_lat = cache_.getInterconnect()->getLatencyForMessage(msg);

        // Current time + latency
        doneEvent.start_time = clock_.now();
        doneEvent.timestamp = doneEvent.start_time
                    + net_lat;
        doneEvent.pe_id = id_;
        doneEvent.action = "instruction_complete";
        doneEvent.qos_ = qos_;


        clock_.add_event(doneEvent);

}

void PE::sendMessageToCache(const Message& msg) {
    std::cout << "[PE " << static_cast<int>(id_) << "] Sending message to cache: "
              << messageToString(msg) << std::endl;
    cache_.receiveMessage(msg);
}

void PE::run() {
    while (running_) {
        Event e;
        {
            std::unique_lock<std::mutex> lk(inbox_mtx_);
            inbox_cv_.wait(lk, [this]{ return !inbox_.empty() || !running_; });
            if (!running_) break;
            e = inbox_.front();
            inbox_.pop();
        }

        // shutdown signal
        if (e.action == "shutdown") break;

        // it's time to execute the next instruction
        if (e.action == "execute_instruction") {
            if (!instructionMemory_.hasNext()) {
                clock_.notify_pe_finished(id_);
                continue;
            }

            Instruction instr = instructionMemory_.getNext();
            std::cout << "[PE " << int(id_) << "] Executing instruction "
                      << static_cast<int>(instr.getType()) << "\n";

            switch (instr.getType()) {
                case InstructionType::READ: {
                    ReadMemMessage msg;
                    msg.src = id_;
                    msg.addr = instr.getAddress();
                    msg.size = instr.getSize();
                    msg.qos  = qos_;
                    msg.type = MessageType::READ_MEM;
                    sendMessageToCache(msg);
                    break;
                }
                case InstructionType::WRITE: {
                    WriteMemMessage msg;
                    msg.src = id_;
                    msg.addr = instr.getAddress();
                    msg.data = instr.getData();
                    msg.num_of_cache_lines = instr.getNumLines();
                    msg.start_cache_line = instr.getStartLine();
                    msg.qos  = qos_;
                    msg.type = MessageType::WRITE_MEM;
                    sendMessageToCache(msg);
                    break;
                }
                case InstructionType::INVALIDATE: {
                    BroadcastInvalidateMessage msg;
                    msg.src = id_;
                    msg.src_cache_line = instr.getCacheLine();
                    msg.type = MessageType::BROADCAST_INVALIDATE;
                    sendMessageToCache(msg);
                    break;
                }
                default: break;
            }
            stats_.instructionsExecuted++;

            // schedule completion
            Event done;
            done.timestamp = clock_.now() + 10;
            done.pe_id    = id_;
            done.action   = "instruction_completed";
            done.qos_ = qos_;
            clock_.add_event(done);
        }
        // instruction latency has elapsed
        else if (e.action == "instruction_completed") {
            onEvent(e);

            // immediately schedule next execute
            Event next;
            next.timestamp = e.timestamp;
            next.pe_id    = id_;
            next.action   = "execute_instruction";
            next.qos_ = qos_;
            clock_.add_event(next);
        }
    }
}

void PE::onEvent(const Event& event) {
    if (event.action == "instruction_completed") {
        std::lock_guard<std::mutex> lk(cout_mutex);
        std::cout << "[PE " << int(id_) << "] Instruction done at T="
                  << event.timestamp << "\n";
    }
}


PE::Statistics PE::getStatistics() const {
    return stats_;
}

uint8_t PE::getPE_id() const {
    return id_;
}

uint8_t PE::getPE_qos() const {
    return qos_;
}


Cache& PE::getCache() {
    return cache_;
}