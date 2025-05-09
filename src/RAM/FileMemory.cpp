#include "../include/RAM/FileMemory.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

FileMemory::FileMemory(const std::string& file) : filename(file) {
    loadFromFile();
}

void FileMemory::loadFromFile() {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string addr_str, value_str;

        if (std::getline(ss, addr_str, ':') && std::getline(ss, value_str)) {
            uint32_t addr = std::stoul(addr_str, nullptr, 16);
            uint8_t value = std::stoul(value_str, nullptr, 16);
            memory[addr] = value;
        }
    }
}

void FileMemory::writeToFile() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[RAM] ERROR: Cannot open file: " << filename << "\n";
        return;
    }
    std::cout << "[DEBUG] Dumping memory contents:\n";
    for (const auto& [addr, value] : memory) {
        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << addr
                  << ": " << std::hex << std::setw(2) << std::setfill('0') << (int)value << '\n';
    }
    for (const auto& [addr, value] : memory) {
        file << "0x" << std::hex << std::setw(8) << std::setfill('0') << addr
             << ": " << std::hex << std::setw(2) << std::setfill('0') << (int)value << '\n';
    }
}


void FileMemory::write(uint32_t addr, const std::vector<uint8_t>& data) {
    // writeToFile();

    std::cout << "[DEBUG] FileMemory::write called. Addr: 0x"
              << std::hex << addr << " | Data size: " << std::dec << data.size() << "\n";

    if (data.empty()) {
        std::cout << "[DEBUG] Warning: data vector is empty!\n";
    }

    std::cout << "[DEBUG] Data bytes: ";
    for (uint8_t b : data) {
        std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::endl;



    for (size_t i = 0; i < data.size(); ++i) {
        memory[addr + i] = data[i];
        std::cout << "[DEBUG] Writing byte at 0x" << std::hex << (addr + i)
                 << " = " << std::setw(2) << std::setfill('0') << (int)data[i] << "\n";
    }

    writeToFile();
}

std::vector<uint8_t> FileMemory::read(uint32_t addr, size_t size) {
    std::lock_guard<std::mutex> lock(mem_mutex); // 🔐 mutex lock

    std::vector<uint8_t> result(size, 0);
    for (size_t i = 0; i < size; ++i) {
        uint32_t currAddr = addr + i;

        if (memory.count(currAddr)) {
            result[i] = memory[currAddr];
            std::cout << "[DEBUG] Read byte at 0x" << std::hex << currAddr
                      << " = 0x" << std::setw(2) << std::setfill('0') << (int)memory[currAddr] << "\n";
        } else {
            std::cout << "[DEBUG] Read byte at 0x" << std::hex << currAddr
                      << " = NOT FOUND (default 0)\n";
        }
    }
    return result;
}


