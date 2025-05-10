#include "../../include/Interconnect/Interconnect.h"
#include "../../include/PE/PE.h"
#include "../../include/Global/Global.h"
#include <iostream>
#include "../../include/RAM/FileMemory.h"
#include <iomanip> //for debugging (prints)

// Registers a PE into the interconnect
void Interconnect::register_cache(uint8_t cache_id, Cache* cache) {
    caches_[cache_id] = cache; // cache pointer
    cache->setInterconnect(this);
}

//El interconnect recibe un request y realiza una acción en función del tipo de request
//ReadMem: Un PE envia una solicitud para traer datos de memoria y escribirlos en cache
// --> El interconnect responde READ_RESP como confirmación al caché del PE solicitante
//WriteMem: Un PE envia un request para escribir a memoria lo que tiene en cache
// --> El interconnect responde WRITE_RESP como confirmación al caché del PE solicitante
//BroadcastInvalidate: Un PE solicitó invalidar una linea de cache, por lo que el Interconnect difunde al resto de PEs
// --> El interconnect envía BROADCAST_INVALIDATE a todos los cachés registrados, Espera a que estos respondan INV_ACK cuando invalidan su linea de cache
// --> El interconnect envía un INV_COMPLETE una vez recibidos todos los acknowledge al PE solicitante
//

Interconnect::Interconnect() : memory("RAM/memory.mif") {
}

void Interconnect::sendMessage(const Message& msg) {

}

void Interconnect::receiveMessage(const Message &msg) {
    std::cout << "El interconnect está recibiendo el mensaje: \n" ;
    std::cout << messageToString(msg) ;
    MessageType type = getMessageType(msg);

    switch (type) {
        case MessageType::READ_MEM: {
            const auto& readMsg = std::get<ReadMemMessage>(msg);

            // Make sure address is valid before accessing memory
            std::cout << "[Interconnect] Attempting READ from 0x" << std::hex << readMsg.addr
                      << " (" << std::dec << readMsg.size << " bytes)" << std::endl;

            std::vector<uint32_t> data = memory.read(readMsg.addr, readMsg.size);

            std::cout << "[Interconnect] READ from 0x" << std::hex << readMsg.addr
                      << " (" << std::dec << readMsg.size << " bytes): ";
            for (uint8_t byte : data) {
                std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)byte << " ";
            }
            std::cout << std::dec << std::endl;

            // Get source PE ID for response
            uint8_t dest = readMsg.src;

            // Check if destination is registered
            if (caches_.find(dest) == caches_.end()) {
                std::cerr << "[Interconnect] ERROR: Cache ID " << (int)dest << " not registered\n";
                return;
            }

            // Create response message
            ReadRespMessage resp;
            resp.src = readMsg.src;  //PE source
            resp.dest = dest;
            resp.qos = readMsg.qos;
            resp.data = data;
            resp.type = MessageType::READ_RESP;  // Ensure type is set correctly

            std::cout << "[Interconnect] Sending response: " << messageToString(resp) << std::endl;

            // Send message to destination cache
            caches_[dest]->receiveMessage(resp);
            break;
        }
            //WRITE_MEM
        case MessageType::WRITE_MEM: {
            WriteRespMessage resp;
            const auto& writeMsg = std::get<WriteMemMessage>(msg);  //

            uint32_t addr = getMessageAddress(msg);
            const std::vector<uint32_t>& data = getMessageData(msg);

            //cambiar esto
            if (memory.canWrite(addr, data.size())) {
                memory.write(addr, data);
                resp.status = 0x0;
            } else {
                resp.status = 0x1;
            }

            // Enviar el mensaje de respuesta
            sendMessage(resp);
            break;

        }


        default:
            std::cerr << "[Interconnect] Unsupported message type\n";
            break;
    }
}



// Called by a Cac when they want to send a message
void Interconnect::send(uint8_t src_pe, const Message& msg) {
    // Many PEs may send concurrently
    std::lock_guard<std::mutex> lock(fifo_mutex_);

    uint64_t latency = getLatencyForMessage(msg);

    // Adds queued messsage to end queue
    fifo_.push(msg);

    Event ev;
    ev.timestamp = clock_->now() + latency;
    ev.pe_id = src_pe;
    ev.action = "interconnect_process";

    // Queues event
    clock_->add_event(ev);
}

// Scheduled Interconnect event to process and forward messages when latency expires
void Interconnect::process_next() {
    std::lock_guard<std::mutex> lock(fifo_mutex_);

    if (fifo_.empty()) return;

    Message msg = fifo_.front();

    // Removes the first element in queue fifo_
    fifo_.pop();

    {
        // Show when message is processed in simulation time
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "[Interconnect] Processed message from PE "
                  << static_cast<int>(getMessageSource(msg)) << " at time "
                  << clock_->now() << "\n";
    }

    MessageType type = getMessageType(msg);

    // Should manage memory access

    switch (type)
    {
        case MessageType::WRITE_MEM:

        case MessageType::READ_MEM: {
            for (const auto& [cache_id, cache_ptr] : caches_)
            {
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << "[PE Owner] " << cache_ptr->getPEOwner()->getPE_id() << "\n";
                cache_ptr->receiveMessage(msg);
            }
            break;
        }

        case MessageType::BROADCAST_INVALIDATE:
            {
                // Broadcast to all except source
                // Cache invalidation: send to everyone except self
                for (const auto& [cache_id, cache_ptr] : caches_)
                {
                    std::lock_guard<std::mutex> cout_lock(cout_mutex);
                    std::cout << "[TEST] " << cache_ptr->getPEOwner()->getPE_id() << "\n";
                    if (1 == getMessageSource(msg)) continue; // If it is the source iterate again for all others
                    cache_ptr->receiveMessage(msg);
                }
                break;
            }


        case MessageType::INV_ACK:
        case MessageType::INV_COMPLETE:
        case MessageType::READ_RESP:
        case MessageType::WRITE_RESP:
            {
                uint8_t dest = getMessageDestination(msg);
                // Looks up destination PE
                if (caches_.count(dest)) {
                    caches_[dest]->receiveMessage(msg); // If exists, destination receives message for aknowledgement
                }
            }
            break;
    }
}

uint64_t Interconnect::getLatencyForMessage(const Message& msg) {
    MessageType type = getMessageType(msg);

    switch (type) {
        case MessageType::WRITE_MEM:
        case MessageType::READ_MEM:
            return 50;

        case MessageType::BROADCAST_INVALIDATE:
            return 20;

        case MessageType::INV_ACK:
        case MessageType::INV_COMPLETE:
            return 10;

        case MessageType::READ_RESP:
            return 40;

        case MessageType::WRITE_RESP:
            return 30;
    }

    return 50;
}

// Extracts dest field from InvCompleteMessage, ReadRespMessage, WriteRespMessage
uint8_t Interconnect::getMessageDestination(const Message &msg) {
    if (std::holds_alternative<InvCompleteMessage>(msg))
        return std::get<InvCompleteMessage>(msg).dest;
    if (std::holds_alternative<ReadRespMessage>(msg))
        return std::get<ReadRespMessage>(msg).dest;
    if (std::holds_alternative<WriteRespMessage>(msg))
        return std::get<WriteRespMessage>(msg).dest;

    return 0xFF; // No destination
}
