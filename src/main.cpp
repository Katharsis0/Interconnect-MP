#include "MESI/MESIProtocol.h"
#include <iostream>

int main() {
    MESIProtocol proto(0);
    uint32_t addr = 0xABC;

    std::cout << "Estado tras lectura: ";
    auto state = proto.handleRead(addr);
    std::cout << static_cast<int>(state) << "\n"; // Debería ser Exclusive (2)

    std::cout << "Estado tras escritura: ";
    state = proto.handleWrite(addr);
    std::cout << static_cast<int>(state) << "\n"; // Debería ser Modified (3)

    proto.invalidate(addr);
    std::cout << "Estado tras invalidación: ";
    state = proto.getState(addr);
    std::cout << static_cast<int>(state) << "\n"; // Debería ser Invalid (0)

    return 0;
}
