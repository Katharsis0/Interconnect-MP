#ifndef FILE_MEMORY_H
#define FILE_MEMORY_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>

class FileMemory {
private:
    std::string filename;
    std::map<uint32_t, uint8_t> memory;

    void loadFromFile();
    void writeToFile();

public:
    FileMemory(const std::string& file);

    void write(uint32_t addr, const std::vector<uint8_t>& data);
    std::vector<uint8_t> read(uint32_t addr, size_t size);
};

#endif // FILE_MEMORY_H
