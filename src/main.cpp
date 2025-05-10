#include "../include/Clock/EventClock.h"
#include "../include/Interconnect/Interconnect.h"
#include "../include/PE/PE.h"
#include "../include/PE/Instruction.h"
#include "../include/Global/Global.h"
#include <vector>
#include <memory>
#include <thread>
#include <iostream>

int main(int argc, char* argv[]) {
    ClockMode mode = ClockMode::Running;
    if (argc > 1 && std::string(argv[1]) == "step")
        mode = ClockMode::Stepping;

    constexpr int NUM_PES = 8;
    EventClock clock(mode);
    Interconnect interconnect;
    interconnect.set_clock(&clock);
    clock.set_interconnect(&interconnect);
    clock.set_total_pes(NUM_PES);

    std::vector<Instruction> program = {
        {InstructionType::READ,  0x08, 4},
        {InstructionType::READ,  0x01, 4},
        {InstructionType::READ,  0x02, 4}
    };

    std::vector<std::unique_ptr<PE>> pes;
    for (int i = 0; i < NUM_PES; ++i) {
        auto pe = std::make_unique<PE>(i, i, clock);
        pe->loadInstructions(program);

        // register handler
        clock.register_pe(i, [pe_ptr = pe.get()](const Event& e) {
            pe_ptr->enqueueEvent(e);
        });
        interconnect.register_cache(i, &pe->getCache());
        pe->start();
        pes.push_back(std::move(pe));

        // seed first execute
        clock.add_event({.timestamp=0, .pe_id=i, .action="execute_instruction", .qos_=255});
    }

    std::thread input_thread;
    if (clock.get_mode() == ClockMode::Stepping) {
        input_thread = std::thread([&] {
            {
                std::lock_guard<std::mutex> lk(clock.step_mutex_);
                clock.input_thread_ready_ = true;
            }
            clock.input_ready_cv_.notify_one();
            std::string line;
            while (std::getline(std::cin, line)) {
                if (!line.empty() && line[0] == 'q') {
                    clock.stop();
                    break;
                }
                {
                    std::lock_guard<std::mutex> lk(clock.step_mutex_);
                    clock.step_ready_ = true;
                }
                clock.step_cv_.notify_one();
            }
        });
    } else {
        std::lock_guard<std::mutex> lk(clock.step_mutex_);
        clock.input_thread_ready_ = true;
        clock.input_ready_cv_.notify_one();
    }

    std::thread clk_thread([&] { clock.run(); });

    clock.wait_until_all_pes_finished();
    clock.stop();
    clk_thread.join();
    if (input_thread.joinable()) input_thread.join();

    // print final statistics and durations
    for (int i = 0; i < NUM_PES; ++i) {
        auto s = pes[i]->getStatistics();
        std::cout << "PE " << i << ": " << s.instructionsExecuted
                  << " instructions executed\n";
    }

    return 0;
}
