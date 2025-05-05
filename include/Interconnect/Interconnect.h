//
// Created by katharsis on 5/2/25.
//

#ifndef INTERCONNECT_H
#define INTERCONNECT_H


#pragma once

#include <vector>
#include <queue>
#include <memory>
#include "../Messages/Messages.h"

class PE;
class MainMemory;

enum class ArbitrationPolicy {
    FIFO,
    QoS_PRIORITY
};

class Interconnect {
private:
    std::vector<PE*> pes;
    MainMemory* memory;
    ArbitrationPolicy policy;

    //FIFO queue
    std::queue<Message> fifo_queue;

    //QoS priority
    struct MessageCompare {
        bool operator()(const Message& a, const Message& b) {
            return a.qos < b.qos; //Higher QoS first
        }
    };
    std::priority_queue<Message, std::vector<Message>, MessageCompare> priority_queue;

    //Statistics
    size_t total_messages;
    size_t total_bytes;

    //Process message
    void processMessage(const Message& msg);

public:
    Interconnect(MainMemory* mem, ArbitrationPolicy pol = ArbitrationPolicy::FIFO);

    void registerPE(PE* pe);
    void sendMessage(const Message& msg);

    //Run the interconnect for one cycle
    void cycle();

    //Get statistics
    void printStatistics() const;
};



#endif //INTERCONNECT_H
