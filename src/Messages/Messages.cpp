//
// Created by katharsis on 5/5/25.
//
#include "Messages/Messages.h"

#include <sstream>

std::string WriteMemMessage::toString() const {
    std::stringstream ss;
    ss << "WRITE_MEM addr=0x" << std::hex << addr << " cache_lines=" << std::dec << (int)num_of_cache_lines;
    return ss.str();
}

std::string ReadMemMessage::toString() const {
    std::stringstream ss;
    ss << "READ_MEM addr=0x" << std::hex << addr << " size=" << std::dec << size;
    return ss.str();
}

std::string BroadcastInvalidateMessage::toString() const {
    std::stringstream ss;
    ss << "BROADCAST_INVALIDATE src_cache_line=" << std::dec << src_cache_line;
    return ss.str();
}

std::string InvAckMessage::toString() const {
    return "INV_ACK";
}

std::string InvCompleteMessage::toString() const {
    std::stringstream ss;
    ss << "INV_COMPLETE dest=" << std::dec << (int)dest;
    return ss.str();
}

std::string ReadRespMessage::toString() const {
    std::stringstream ss;
    ss << "READ_RESP dest=" << std::dec << (int)dest << " data_size=" << data.size();
    return ss.str();
}

std::string WriteRespMessage::toString() const {
    std::stringstream ss;
    ss << "WRITE_RESP dest=" << std::dec << (int)dest << " status=" << (status == 1 ? "OK" : "NOT_OK");
    return ss.str();
}
