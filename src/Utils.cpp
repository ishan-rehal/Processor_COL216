#include "Utils.hpp"
#include <fstream>
#include <iostream>

// namespace Utils {
//     std::vector<std::string> readInstructionsFromFile(const std::string& filename) {
//         std::vector<std::string> instructions;
//         std::ifstream infile(filename);
//         if (!infile) {
//             std::cerr << "Error opening file: " << filename << std::endl;
//             return instructions;
//         }
//         std::string line;
//         while (std::getline(infile, line)) {
//             if (!line.empty())
//                 instructions.push_back(line);
//         }
//         infile.close();
//         return instructions;
//     }
// }


#include <sstream>  // for std::istringstream

namespace Utils {
    std::vector<std::string> readInstructionsFromFile(const std::string& filename) {
        std::vector<std::string> instructions;
        std::ifstream infile(filename);
        if (!infile) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return instructions;
        }

        std::string line;
        while (std::getline(infile, line)) {
            // Skip empty lines
            if (line.empty()) continue;

            // Use a string stream to parse the line by whitespace
            std::istringstream iss(line);
            std::string hexCode;
            if (iss >> hexCode) {
                // hexCode is the first token in the line (e.g. "00000293")
                instructions.push_back(hexCode);
            }
        }

        infile.close();
        return instructions;
    }
}

