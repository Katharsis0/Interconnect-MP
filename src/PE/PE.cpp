#include "../../include/PE/PE.h"
#include "../../include/Memory/MemorySlave.h"
#include <iostream>
#include <sstream>
#include <algorithm>

PE::PE(int id, const std::vector<std::string>& instructions, MemorySlave* memSlave)
    : id(id), pc(0), instructionMemory(instructions), memSlave(memSlave), cache(id) {
    std::cout << "PE creado con ID: " << id << std::endl;
}

void PE::getState() const {
    std::cout << "Estado del CPU " << id << ":\n";
    std::cout << "Program Counter (PC): " << pc << "\n";
}

bool PE::nextInstruction() {
    if (pc < instructionMemory.size()) {
        executeInstruction(instructionMemory[pc]);
        pc++;
        return true;
    }
    return false;
}

Message PE::parseInstruction(const std::string& instruction) {
    std::istringstream iss(instruction);
    std::string op;
    iss >> op;

    Message msg(MessageType::WRITE_MEM, id, 0);

    if (op == "WRITE_MEM") {
        msg.type = MessageType::WRITE_MEM;
        uint32_t addr;
        size_t num_lines, start_line;
        iss >> addr >> num_lines >> start_line >> msg.qos;

        //Read data from cache
        for (size_t i = 0; i < num_lines; i++) {
            uint32_t line_addr = addr + (start_line + i) * 16;
            auto data = cache.read(line_addr, 16);
            msg.data.insert(msg.data.end(), data.begin(), data.end());
        }
    }
    else if (op == "READ_MEM") {
        msg.type = MessageType::READ_MEM;
        iss >> msg.addr >> msg.size >> msg.qos;
    }
    else if (op == "BROADCAST_INVALIDATE") {
        msg.type = MessageType::BROADCAST_INVALIDATE;
        iss >> msg.cache_line >> msg.qos;
    }
    else if (op == "INV_ACK") {
        msg.type = MessageType::INV_ACK;
        iss >> msg.src >> msg.qos;
    }
    else if (op == "INV_COMPLETE") {
        msg.type = MessageType::INV_COMPLETE;
        iss >> msg.dest >> msg.qos;
    }
    else if (op == "READ_RESP") {
        msg.type = MessageType::READ_RESP;
        iss >> msg.dest >> msg.data >> msg.qos;
    }else if (op == "WRITE_RESP") {
        msg.type = MessageType::WRITE_RESP;
        iss >> msg.dest >> msg.status >> msg.qos;
    }

    return msg;
}

void PE::executeInstruction(const std::string& instruction) {
    Message msg = parseInstruction(instruction);

}

void PE::receiveMessage(const Message& msg) {
    switch(msg.type) {
        case MessageType::READ_RESP:
            // Store data in cache
            cache.write(msg.addr, msg.data);
            break;

        case MessageType::WRITE_RESP:
            // Handle write confirmation
            if (msg.status != 0x1) {
                std::cerr << "PE " << id << ": Write operation failed\n";
            }
            break;

        case MessageType::BROADCAST_INVALIDATE:
            // Invalidate cache line
            cache.invalidate(msg.cache_line);

            // Send acknowledgment
            Message ack(MessageType::INV_ACK, id, msg.qos);
            break;
       //Faltan los otros mensajes
    }
}