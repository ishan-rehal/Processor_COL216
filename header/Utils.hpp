#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

namespace Utils {
    // Reads instructions (8-character hex strings) from a file.
    std::vector<std::string> readInstructionsFromFile(const std::string& filename);
}

#endif // UTILS_HPP
