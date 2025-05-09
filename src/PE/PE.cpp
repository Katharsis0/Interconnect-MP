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

        // Current time + latency
        doneEvent.timestamp = clock_.now() + 10; // Schedule this event to happen 10 logical time units after now
        doneEvent.pe_id = id_;
        doneEvent.action = "instruction_done";

        clock_.add_event(doneEvent);

}

void PE::sendMessageToCache(const Message& msg) {
    std::cout << "[PE " << static_cast<int>(id_) << "] Sending message to cache: "
              << messageToString(msg) << std::endl;
    cache_.receiveMessage(msg);
}

void PE::run() {
    while (running_ && instructionMemory_.hasNext()) {
        Instruction instr = instructionMemory_.getNext();

        {
            std::cout << "[PE " << static_cast<int>(id_) << "] Executing instruction\n";
        }

        switch (instr.getType()) {
            case InstructionType::READ: {
                {
                    std::cout << "[PE " << static_cast<int>(id_) << "] La instrucción a ejecutar es READ\n";
                }

                ReadMemMessage read;
                read.src = id_;
                read.addr = instr.getAddress();
                read.size = instr.getSize();
                read.qos = qos_;
                read.type = MessageType::READ_MEM;
                sendMessageToCache(read);
                break;
            }

            case InstructionType::WRITE: {
                {
                    std::cout << "[PE " << static_cast<int>(id_) << "] La instrucción a ejecutar es WRITE\n";
                }

                WriteMemMessage write;
                write.src = id_;
                write.addr = instr.getAddress();
                write.data = instr.getData();
                write.num_of_cache_lines = instr.getNumLines();
                write.start_cache_line = instr.getStartLine();
                write.qos = qos_;
                write.type = MessageType::WRITE_MEM;
                sendMessageToCache(write);
                break;
            }

            case InstructionType::INVALIDATE: {
                {
                    std::cout << "[PE " << static_cast<int>(id_) << "] La instrucción a ejecutar es INVALIDATE\n";
                }

                BroadcastInvalidateMessage inv;
                inv.src = id_;
                inv.src_cache_line = instr.getCacheLine();
                inv.type = MessageType::BROADCAST_INVALIDATE;
                sendMessageToCache(inv);
                break;
            }

            default:
                std::cerr << "[PE " << static_cast<int>(id_) << "] Unknown instruction type\n";
                break;
        }

        stats_.instructionsExecuted++;

        // Esperar a que se dispare el evento "instruction_done"
        std::unique_lock<std::mutex> lock(pe_mutex_);
        pe_cv_.wait(lock);
    }

    // Notificar que este PE terminó
    clock_.notify_pe_finished(id_);
}



void PE::onEvent(const Event& event) {
    if (event.action == "instruction_done") {
        {
            std::lock_guard<std::mutex> cout_lock(cout_mutex);
            std::cout << "PE " << static_cast<int>(id_) << " reanuda ejecución\n";
        }

        {
            std::lock_guard<std::mutex> lock(pe_mutex_);
            pe_cv_.notify_all();
        }
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