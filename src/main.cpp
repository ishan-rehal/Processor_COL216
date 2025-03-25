#include <iostream>
#include <vector>
#include <string>
#include "Processor.hpp"
#include "Utils.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <cycle_count> [forward]" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    int cycleCount = std::stoi(argv[2]);
    bool forwarding = false;
    if (argc > 3) {
        std::string mode = argv[3];
        if (mode == "forward") {
            forwarding = true;
        }
    }

    // Read instructions from file.
    std::vector<std::string> instructions = Utils::readInstructionsFromFile(inputFile);

    // Create Processor instance.
    Processor processor(instructions, forwarding, cycleCount);

    // Run simulation for the specified number of cycles.
    for (int cycle = 0; cycle < cycleCount; ++cycle) {
        // std::cout << "Cycle " << cycle + 1 << ":\n";
        processor.runCycle();
        
        // The runCycle() method internally updates and prints pipeline state.
        // If you want to print it again separately, uncomment the next line.
        // processor.printPipelineState();
        // std::cout << "-----------------------\n";
    }
    processor.printFullPipelineLog();

    return 0;
}
