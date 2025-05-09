#include "../include/Clock/EventClock.h"
#include "../include/PE/PE.h"
#include "../include/PE/Instruction.h"
#include "../include/Interconnect/Interconnect.h"
#include "../include/Global/Global.h"

#include <vector>
#include <memory>
#include <thread>
#include <iostream>
#include <mutex>
#include <string>

int main(int argc, char* argv[]) {
    ClockMode mode = ClockMode::Running;
    if (argc > 1 && std::string(argv[1]) == "step") {
        mode = ClockMode::Stepping;
    }

    constexpr int NUM_PES = 2;
    std::cout << "=== Starting simulation with " << NUM_PES << " PE(s) ===" << std::endl;

    EventClock clock(mode);
    Interconnect interconnect;
    interconnect.set_clock(&clock);
    clock.set_interconnect(&interconnect);
    clock.set_total_pes(NUM_PES);

    std::vector<Instruction> program = {
        Instruction(InstructionType::READ, 0x00000008, 4),
        Instruction(InstructionType::READ, 0x00000001, 4),
        Instruction(InstructionType::READ, 0x00000002, 4)
    };

    std::vector<std::unique_ptr<PE>> pes;

    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, 1, clock);
        pe->loadInstructions(program);

        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->onEvent(e);
        });

        interconnect.register_cache(i, &pe->getCache());
        pes.push_back(std::move(pe));
    }


    std::thread input_thread;
    if (clock.get_mode() == ClockMode::Stepping) {
        input_thread = std::thread([&]() {
            {
                std::lock_guard<std::mutex> lock(clock.input_ready_mutex_);
                clock.input_thread_ready_ = true;
            }
            clock.input_ready_cv_.notify_one();

            std::string input;
            while (true) {
                std::getline(std::cin, input);

                if (!input.empty() && input[0] == 'q') {
                    clock.stop();
                    break;
                }

                {
                    std::lock_guard<std::mutex> lock(clock.step_mutex_);
                    clock.step_ready_ = true;
                }
                clock.step_cv_.notify_one();
            }
        });
    } else {
        // Si no está en modo Stepping, marcar como listo inmediatamente
        {
            std::lock_guard<std::mutex> lock(clock.input_ready_mutex_);
            clock.input_thread_ready_ = true;
        }
        clock.input_ready_cv_.notify_one();
    }

    std::thread clock_thread([&]() { clock.run(); });
    for (auto& pe : pes) pe->start();

    clock.wait_until_all_pes_finished();
    clock.stop();
    clock_thread.join();

    for (auto& pe : pes) pe->stop();

    if (input_thread.joinable())
        input_thread.join();

    for (size_t i = 0; i < pes.size(); ++i) {
        auto stats = pes[i]->getStatistics();
        std::lock_guard<std::mutex> cout_lock(cout_mutex);
        std::cout << "PE " << i << ": " << stats.instructionsExecuted << " instrucciones ejecutadas\n";
    }

    return 0;
}
