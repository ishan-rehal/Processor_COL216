// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include "Processor.hpp"
#include "Utils.hpp"

int main(int argc, char* argv[]) {
    // Default forwarding value is determined by compile-time flag.
    bool forwarding = false;
#ifdef FORWARDING
    forwarding = true;
#endif

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <cycle_count> [forward]" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    int cycleCount = std::stoi(argv[2]);

    // Allow runtime override if provided.
    if (argc > 3) {
        std::string mode = argv[3];
        if (mode == "forward") {
            forwarding = true;
        } else {
            forwarding = false;
        }
    }

    // Read instructions from file.
    std::vector<std::string> instructions = Utils::readInstructionsFromFile(inputFile);

    // Create Processor instance.
    Processor processor(instructions, forwarding, cycleCount);

    // Run simulation for the specified number of cycles.
    for (int cycle = 0; cycle < cycleCount; ++cycle) {
        processor.runCycle();
    }
    processor.printFullPipelineLog();
    processor.print_registers();
    std::cout << "Forwarding enabled: " << (forwarding ? "true" : "false") << std::endl;
    return 0;
}
