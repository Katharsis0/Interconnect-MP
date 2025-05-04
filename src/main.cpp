#include "../include/Clock/EventClock.h"
#include "../include/Clock/Scheduler.h"
#include "../include/PE/PE.h"
#include "../include/PE/Instruction.h"
#include "../include/Interconnect/Interconnect.h"
#include "../include/Global/Global.h"


#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <iostream>


int main() {
    // Crear reloj global
    EventClock clock;

    // Crear instrucciones de ejemplo
    std::vector<Instruction> program = {
        Instruction(InstructionType::READ,  0x1000),
        Instruction(InstructionType::WRITE, 0x2000),
        Instruction(InstructionType::READ,  0x3000),
        Instruction(InstructionType::WRITE, 0x4000)
    };

    // Crear PE e Interconnect (interconnect aún vacío en este ejemplo)
    Interconnect interconnect;

    // Vector para guardar los PEs
    constexpr int NUM_PES = 8;
    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, 1, &interconnect, clock);
        pe->loadInstructions(program);

        // Registrar el PE en el EventClock
        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->onEvent(e);
        });

        pes.push_back(std::move(pe));
    }

    // Lanzar los PE
    for (auto& pe : pes) {
        pe->start();
    }

    // Lanzar el EventClock en un hilo
    std::thread clock_thread(&EventClock::run, &clock);

    // Esperar que los PE terminen (muy simple → solo esperamos fijo en este ejemplo)
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Parar el reloj de eventos
    clock.stop();
    clock_thread.join();

    // Detener los PEs
    for (auto& pe : pes) {
        pe->stop();
    }

    // Mostrar estadísticas
    for (size_t i = 0; i < pes.size(); ++i) {
        auto stats = pes[i]->getStatistics();
        std::cout << "PE " << i << ": " << stats.instructionsExecuted << " instrucciones ejecutadas\n";
    }

    return 0;
}
