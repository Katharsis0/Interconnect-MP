#include "RAM/FileMemory.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

FileMemory::FileMemory(const std::string& mif_path) : filename(mif_path) {
    loadFromMIF();
}

void FileMemory::loadFromMIF() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[FileMemory] ERROR: Cannot open MIF file: " << filename << std::endl;
        return;
    }
    std::string line;
    bool content_started = false;

    while (std::getline(file, line)) {
        if (line.find("CONTENT BEGIN") != std::string::npos) {
            content_started = true;
            continue;
        }
        if (line.find("END") != std::string::npos) break;
        if (!content_started) continue;

        std::stringstream ss(line);
        std::string addr_str, colon, data_str;
        ss >> addr_str >> colon >> data_str;
        if (colon != ":") continue;

        uint32_t addr = std::stoul(addr_str, nullptr, 16);
        uint32_t data = std::stoul(data_str, nullptr, 16);
        memory[addr] = data;
    }
}

void FileMemory::writeToMIF() {
    std::ofstream file(filename);
    file << "DEPTH = 4096;\nWIDTH = 32;\nADDRESS_RADIX = HEX;\nDATA_RADIX = HEX;\nCONTENT BEGIN\n";

    for (uint32_t i = 0; i < 4096; ++i) {
        uint32_t data = memory.count(i) ? memory[i] : 0;
        file << std::uppercase << std::setfill('0') << std::hex
             << std::setw(4) << i << " : "
             << std::setw(8) << data << ";\n";
    }

    file << "END;\n";
}



std::vector<uint32_t> FileMemory::read(uint32_t addr, size_t size) {
    std::vector<uint32_t> result;
    for (size_t i = 0; i < size; ++i) {
        uint32_t word_addr = (addr + i) / 4;
        uint32_t offset = (addr + i) % 4;
        uint32_t word = memory[word_addr];
        uint8_t byte = (word >> ((3 - offset) * 8)) & 0xFF;
        result.push_back(byte);
    }
    return result;
}

void FileMemory::write(uint32_t addr, const std::vector<uint32_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        uint32_t word_addr = (addr / 4) + i;
        memory[word_addr] = data[i];
    }
}


bool FileMemory::canWrite(uint32_t addr, size_t size) const {
    // Si addr es mayor que el máximo permitido, o el rango [addr, addr+size) se pasa
    if (addr + size > MAX_MEMORY_BYTES) {
        return false;
    }
    return true;
}

/*void FileMemory::write(uint32_t addr, const std::vector<uint8_t>& data) {
    for (size_t i = 0; i < data.size(); ++i) {
        uint32_t byte_addr = addr + i;
        uint32_t word_addr = byte_addr / 4;
        uint32_t offset = (byte_addr % 4);
        uint32_t shift = (3 - offset) * 8;
        uint32_t& word = memory[word_addr];
        word &= ~(0xFF << shift);             // Clear the byte
        word |= static_cast<uint32_t>(data[i]) << shift; // Set the byte
    }

    writeToMIF();
}*/
