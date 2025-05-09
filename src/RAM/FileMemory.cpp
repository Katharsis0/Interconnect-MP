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
    std::ofstream file(filename, std::ios::trunc);
    for (const auto& [addr, value] : memory) {
        file << "0x" << std::hex << std::setw(8) << std::setfill('0') << addr
             << ": " << std::hex << std::setw(2) << std::setfill('0') << (int)value << '\n';
    }
}

void FileMemory::write(uint32_t addr, const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        memory[addr + i] = data[i];
    }
    writeToFile();
}

std::vector<uint8_t> FileMemory::read(uint32_t addr, size_t size) {
    std::vector<uint8_t> result(size, 0);
    for (size_t i = 0; i < size; ++i) {
        if (memory.count(addr + i)) {
            result[i] = memory[addr + i];
        }
    }
    return result;
}
