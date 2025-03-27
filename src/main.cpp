// // main.cpp
// #include <iostream>
// #include <vector>
// #include <string>
// #include "Processor.hpp"
// #include "Utils.hpp"

// int main(int argc, char* argv[]) {
//     // Default forwarding value is determined by compile-time flag.
//     bool forwarding = false;
// #ifdef FORWARDING
//     forwarding = true;
// #endif

//     if (argc < 3) {
//         std::cerr << "Usage: " << argv[0] << " <input_file> <cycle_count> [forward]" << std::endl;
//         return 1;
//     }

//     std::string inputFile = argv[1];
//     int cycleCount = std::stoi(argv[2]);

//     // Allow runtime override if provided.
//     if (argc > 3) {
//         std::string mode = argv[3];
//         if (mode == "forward") {
//             forwarding = true;
//         } else {
//             forwarding = false;
//         }
//     }

//     // Read instructions from file.
//     std::vector<std::string> instructions = Utils::readInstructionsFromFile(inputFile);

//     // Create Processor instance.
//     Processor processor(instructions, forwarding, cycleCount);

//     // Run simulation for the specified number of cycles.
//     for (int cycle = 0; cycle < cycleCount; ++cycle) {
//         std :: cout << "Cycle No: " << cycle + 1 << std::endl;
//         processor.runCycle();
//     }
//     processor.printFullPipelineLog();
//     processor.print_registers();
//     std::cout << "Forwarding enabled: " << (forwarding ? "true" : "false") << std::endl;
//     return 0;
// }



#include <iostream>
#include <fstream>
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

    // Open "output.txt" in the current directory (src).
    // All std::cout output will be redirected to this file.
    std::ofstream outFile("../outputfiles/output.txt");
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output.txt for writing.\n";
        return 1;
    }
    // Backup the old buffer and redirect std::cout to output.txt
    std::streambuf* oldCoutBuf = std::cout.rdbuf(outFile.rdbuf());

    // Read instructions from file.
    std::vector<std::string> instructions = Utils::readInstructionsFromFile(inputFile);
    std::vector<std::string> asmStatements = Utils::readAssemblyStatementsFromFile(inputFile);

    // Create Processor instance.
    Processor processor(instructions, forwarding, cycleCount,asmStatements);

    // Run simulation for the specified number of cycles.
    for (int cycle = 0; cycle < cycleCount; ++cycle) {
        // std::cout << "Cycle No: " << cycle + 1 << std::endl;
        processor.runCycle();
    }
    // processor.printFullPipelineLog();
    processor.printFullPipelineLogSimple();
    // processor.print_registers();
    // std::cout << "Forwarding enabled: " << (forwarding ? "true" : "false") << std::endl;

    // Restore std::cout to its old buffer
    std::cout.rdbuf(oldCoutBuf);

    return 0;
}
