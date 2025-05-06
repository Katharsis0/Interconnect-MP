#include "../../include/Interconnect/Interconnect.h"
#include "../../include/PE/PE.h"
#include "../../include/Global/Global.h"
#include <iostream>

Interconnect::Interconnect(EventClock* clock)
    : clock_(clock) {}

// Registers a PE into the interconnect
void Interconnect::register_cache(uint8_t cache_id, Cache* cache) {
    caches_[cache_id] = cache; // pe pointer
}

// Called by a PE when they want to send a message
void Interconnect::send(uint8_t src_pe, const Message& msg) {
    // Many PEs may send concurrently
    std::lock_guard<std::mutex> lock(fifo_mutex_);

    QueuedMessage qm;
    qm.msg = msg;
    qm.src_pe = src_pe;

    uint64_t latency = getLatencyForMessage(msg);
    qm.scheduled_time = clock_->now() + latency;

    // Adds queued messsage to end queue
    fifo_.push(qm);

    Event ev;
    ev.timestamp = qm.scheduled_time;
    ev.pe_id = src_pe;
    ev.action = "interconnect_process";

    // Queues event
    clock_->add_event(ev);
}

// Scheduled Interconnect event to process and forward messages when latency expires
void Interconnect::process_next() {
    std::lock_guard<std::mutex> lock(fifo_mutex_);

    if (fifo_.empty()) return;

    QueuedMessage qm = fifo_.front();

    // Removes the first element in queue fifo_
    fifo_.pop();

    {
        // Show when message is processed in simulation time
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "[Interconnect] Processed message from PE "
                  << static_cast<int>(qm.src_pe) << " at time "
                  << clock_->now() << "\n";
    }

    MessageType type = getMessageType(qm.msg);

    // Should manage memory access

    switch (type)
    {
        case MessageType::WRITE_MEM:

        case MessageType::READ_MEM:
            {
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << "-> [Interconnect] Forward to Memory ***\n";
                break;
            }

        case MessageType::BROADCAST_INVALIDATE:
            {
                // Broadcast to all except source
                // Cache invalidation: send to everyone except self
                for (const auto& [cache_id, cache_ptr] : caches_)
                {
                    std::lock_guard<std::mutex> cout_lock(cout_mutex);
                    std::cout << "[TEST] " << cache_ptr->getPE()->getPE_id() << "\n";
                    if (1 == qm.src_pe) continue; // If it is the source iterate again for all others
                    cache_ptr->receiveMessagePE(qm.msg);
                }
                break;
            }


        case MessageType::INV_ACK:
        case MessageType::INV_COMPLETE:
        case MessageType::READ_RESP:
        case MessageType::WRITE_RESP:
            {
                uint8_t dest = getMessageDestination(qm.msg);
                // Looks up destination PE
                if (caches_.count(dest)) {
                    caches_[dest]->receiveMessagePE(qm.msg); // If exists, destination receives message for aknowledgement
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
uint8_t Interconnect::getMessageDestination(const Message& msg) {
    if (std::holds_alternative<InvCompleteMessage>(msg))
        return std::get<InvCompleteMessage>(msg).dest;
    if (std::holds_alternative<ReadRespMessage>(msg))
        return std::get<ReadRespMessage>(msg).dest;
    if (std::holds_alternative<WriteRespMessage>(msg))
        return std::get<WriteRespMessage>(msg).dest;

    return 0xFF; // No destination
}
