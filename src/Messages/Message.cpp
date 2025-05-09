//
// Created by katharsis on 4/28/25.
//
#include "Messages/Messages.h"
#include <sstream>

//Cada tipo de mensaje según la espe:
std::string WriteMemMessage::toString() const {
    std::ostringstream oss;
    oss << "WRITE_MEM [SRC:" << (int)src << " QoS:" << (int)qos
        << " ADDR:0x" << std::hex << addr
        << " LINES:" << std::dec << (int)num_of_cache_lines
        << " START:0x" << std::hex << start_cache_line
        << " DATA_SIZE:" << std::dec << data.size() << " bytes]";
    return oss.str();
}


std::string ReadMemMessage::toString() const {
    std::ostringstream oss;
    oss << "READ_MEM [SRC:" << (int)src << " QoS:" << (int)qos
        << " ADDR:0x" << std::hex << addr
        << " SIZE:" << std::dec << size << " bytes]";
    return oss.str();
}

std::string BroadcastInvalidateMessage::toString() const {
    std::ostringstream oss;
    oss << "BROADCAST_INVALIDATE [SRC:" << (int)src << " QoS:" << (int)qos
        << " CACHE_LINE:0x" << std::hex << src_cache_line << "]";
    return oss.str();
}

std::string InvAckMessage::toString() const {
    std::ostringstream oss;
    oss << "INV_ACK [SRC:" << (int)src << " QoS:" << (int)qos << "]";
    return oss.str();
}

std::string InvCompleteMessage::toString() const {
    std::ostringstream oss;
    oss << "INV_COMPLETE [SRC:" << (int)src << " QoS:" << (int)qos
        << " DEST:" << (int)dest << "]";
    return oss.str();
}

std::string ReadRespMessage::toString() const {
    std::ostringstream oss;
    oss << "READ_RESP [SRC:" << (int)src << " QoS:" << (int)qos
        << " DEST:" << (int)dest
        << " DATA_SIZE:" << data.size() << " bytes]";
    return oss.str();
}

std::string WriteRespMessage::toString() const {
    std::ostringstream oss;
    oss << "WRITE_RESP [SRC:" << (int)src << " QoS:" << (int)qos
        << " DEST:" << (int)dest
        << " STATUS:" << (status ? "OK" : "NOT_OK") << "]";
    return oss.str();
}

//Utils
MessageType getMessageType(const Message& msg) {
    return std::visit([](auto&& arg) -> MessageType { return arg.type; }, msg);
}

uint8_t getMessageSource(const Message& msg) {
    return std::visit([](auto&& arg) -> uint8_t { return arg.src; }, msg);
}

uint32_t getMessageAddress(const Message& msg) {
    return std::visit([](auto&& arg) -> uint32_t { return arg.addr; }, msg);
}

size_t getMessageSize(const Message& msg) {
    return std::visit([](auto&& arg) -> size_t { return arg.size; }, msg);
}

std::string messageToString(const Message& msg) {
    return std::visit([](auto&& arg) -> std::string { return arg.toString(); }, msg);
}

size_t calculateMessageSize(const Message& msg) {
    return std::visit([](auto&& arg) -> size_t {
        using T = std::decay_t<decltype(arg)>;

        // Tamaño base (campos comunes)
        size_t size = sizeof(arg.type) + sizeof(arg.src) + sizeof(arg.qos);

        // Campos específicos de cada mensaje
        if constexpr (std::is_same_v<T, WriteMemMessage>) {
            size += sizeof(arg.addr) + sizeof(arg.num_of_cache_lines) +
                   sizeof(arg.start_cache_line) + arg.data.size();
        }
        else if constexpr (std::is_same_v<T, ReadMemMessage>) {
            size += sizeof(arg.addr) + sizeof(arg.size);
        }
        else if constexpr (std::is_same_v<T, BroadcastInvalidateMessage>) {
            size += sizeof(arg.src_cache_line);
        }
        else if constexpr (std::is_same_v<T, InvCompleteMessage>) {
            size += sizeof(arg.dest);
        }
        else if constexpr (std::is_same_v<T, ReadRespMessage>) {
            size += sizeof(arg.dest) + arg.data.size();
        }
        else if constexpr (std::is_same_v<T, WriteRespMessage>) {
            size += sizeof(arg.dest) + sizeof(arg.status);
        }
        // INV_ACK no tiene campos adicionales

        return size;
    }, msg);
}