//
// Created by katharsis on 5/2/25.
//
#include "Interconnect/Interconnect.h"
#include <chrono>
#include <thread>

#include "Memory/MainMemory.h"
#include "PE/PE.h"

Interconnect::Interconnect(MainMemory* mem, ArbitrationPolicy pol)
    : memory(mem), policy(pol), total_messages(0), total_bytes(0) {}

void Interconnect::registerPE(PE* pe) {
    pes.push_back(pe);
}

void Interconnect::sendMessage(const Message& msg) {
    switch(policy) {
        case ArbitrationPolicy::FIFO:
            fifo_queue.push(msg);
            break;
        case ArbitrationPolicy::QoS_PRIORITY:
            priority_queue.push(msg);
            break;
    }
}

void Interconnect::cycle() {
    Message msg(MessageType::WRITE_MEM, 0, 0); //Initial message

    //Next message based on policy
    if (policy == ArbitrationPolicy::FIFO && !fifo_queue.empty()) {
        msg = fifo_queue.front();
        fifo_queue.pop();
    }
    else if (policy == ArbitrationPolicy::QoS_PRIORITY && !priority_queue.empty()) {
        msg = priority_queue.top();
        priority_queue.pop();
    }
    else {
        return; //No messages to process
    }

    //Update statistics
    total_messages++;
    total_bytes += msg.getSize();


    //Process the message
    processMessage(msg);
}

void Interconnect::processMessage(const Message& msg) {
    switch(msg.type) {
        case MessageType::READ_MEM: {
            //Read from memory and send response
            auto data = memory->read(msg.addr, msg.size);
            Message resp(MessageType::READ_RESP, 0, msg.qos);
            resp.dest = msg.src;
            resp.addr = msg.addr;
            resp.data = data;
            pes[msg.src]->receiveMessage(resp);
            break;
        }

        case MessageType::WRITE_MEM: {
            //Write to memory and send response
            uint8_t status = memory->write(msg.addr, msg.data);
            Message resp(MessageType::WRITE_RESP, 0, msg.qos);
            resp.dest = msg.src;
            resp.status = status;
            pes[msg.src]->receiveMessage(resp);
            break;
        }

        case MessageType::BROADCAST_INVALIDATE: {
            // Broadcast to all PEs except sender
            Message ack(MessageType::INV_ACK, 0, msg.qos);
            for (auto* pe : pes) {
                if (pe->getId() != msg.src) {
                    pe->receiveMessage(msg);
                    //Collect ACKs (simplified)
                    ack.src = pe->getId();
                    pes[msg.src]->receiveMessage(ack);
                }
            }
            // Send completion message
            Message complete(MessageType::INV_COMPLETE, 0, msg.qos);
            complete.dest = msg.src;
            pes[msg.src]->receiveMessage(complete);
            break;
        }

        // Other message types can be handled similarly
        default:
            std::cerr << "Unhandled message type: " << static_cast<int>(msg.type) << std::endl;
    }
}

void Interconnect::printStatistics() const {
    std::cout << "Interconnect Statistics:\n";
    std::cout << "Total messages processed: " << total_messages << "\n";
    std::cout << "Total bytes transferred: " << total_bytes << "\n";
    std::cout << "Pending messages in queue: "
              << (policy == ArbitrationPolicy::FIFO ? fifo_queue.size() : priority_queue.size())
              << "\n";
}