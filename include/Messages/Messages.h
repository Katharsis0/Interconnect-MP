#pragma once

#include <vector>
#include <cstdint>

enum class MessageType {
    WRITE_MEM,
    READ_MEM,
    BROADCAST_INVALIDATE,
    INV_ACK,
    INV_COMPLETE,
    READ_RESP,
    WRITE_RESP
};

struct Message {
    MessageType type;
    uint8_t src;        //PE source (0x00-0x07)
    uint8_t dest;       //PE destination
    uint32_t addr;      //Memory address
    uint8_t qos;        //Priority (0x00-0xFF)
    uint16_t size;      //Size in bytes (for READ_MEM)
    uint8_t status;     //WRITE_RESP (0x1: OK, 0x0: NOT_OK)
    uint8_t cache_line; //BROADCAST_INVALIDATE
    std::vector<uint8_t> data; //Payload

    // Constructor for different message types
    Message(MessageType t, uint8_t s, uint8_t q) : type(t), src(s), qos(q) {}

    size_t getSize() const {
        size_t base_size = sizeof(type) + sizeof(src) + sizeof(dest) + sizeof(qos);
        switch(type) {
            case MessageType::WRITE_MEM:
                return base_size + sizeof(addr) + data.size();
            case MessageType::READ_MEM:
                return base_size + sizeof(addr) + sizeof(size);
            case MessageType::BROADCAST_INVALIDATE:
                return base_size + sizeof(cache_line);
            case MessageType::INV_ACK:
                return base_size + sizeof(cache_line);
            case MessageType::INV_COMPLETE:
                return base_size + sizeof(cache_line);
            case MessageType::READ_RESP:
                return base_size + sizeof(cache_line);
            case MessageType::WRITE_RESP:
                return base_size + sizeof(cache_line);
            default:
                return base_size;
        }
    }
};