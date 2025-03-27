#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
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
    // New function: Read and trim the assembly statements.
    std::vector<std::string> readAssemblyStatementsFromFile(const std::string& filename) {
        std::vector<std::string> asmStatements;
        std::ifstream infile(filename);
        if (!infile) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return asmStatements;
        }
        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string hexCode;
            if (iss >> hexCode) {
                std::string rest;
                std::getline(iss, rest);
                // Trim leading whitespace
                rest.erase(rest.begin(), std::find_if(rest.begin(), rest.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
                // Trim trailing whitespace
                rest.erase(std::find_if(rest.rbegin(), rest.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(), rest.end());
                asmStatements.push_back(rest);
            }
        }
        infile.close();
        return asmStatements;
    }
}

