#include "Utils.hpp"
#include <fstream>
#include <iostream>

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
            if (!line.empty())
                instructions.push_back(line);
        }
        infile.close();
        return instructions;
    }
}
