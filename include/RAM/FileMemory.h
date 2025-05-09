#ifndef FILE_MEMORY_H
#define FILE_MEMORY_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>

class FileMemory {
public:
    explicit FileMemory(const std::string& mif_path);
    std::vector<uint8_t> read(uint32_t addr, size_t size);
    void write(uint32_t addr, const std::vector<uint8_t>& data);

private:
    std::string filename;
    std::map<uint32_t, uint32_t> memory; //Palabra de 4 bytes

    void loadFromMIF();
    void writeToMIF();
};

#endif // FILE_MEMORY_H
