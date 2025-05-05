#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Cache.h"
#include "../Messages/Messages.h"

class MemorySlave; // Forward declaration

class PE {
private:
    int id;
    size_t pc;
    std::vector<std::string> instructionMemory;
    MemorySlave* memSlave;
    Cache cache;
    std::vector<Message> pendingResponses;

    
    // Parse instruction string into Message
    Message parseInstruction(const std::string& instruction);
    
    // Handle incoming messages
    void handleMessage(const Message& msg);
    
public:
    PE(int id, const std::vector<std::string>& instructions, MemorySlave* memSlave);
    
    int getId() const { return id; }
    int getPC() const { return pc; }
    
    void getState() const;
    bool nextInstruction();
    void executeInstruction(const std::string& instruction);
    
    // Called by Interconnect when a message arrives
    void receiveMessage(const Message& msg);
};