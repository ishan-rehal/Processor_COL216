#ifndef CONTROLUNIT_HPP
#define CONTROLUNIT_HPP

#include "Instruction.hpp"

// Structure holding the control signals for an instruction.
struct ControlSignals {
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    int aluOp; // For example, 0 = add, 1 = subtract, etc.
};

class ControlUnit {
public:
    // Decodes the instruction and returns its control signals.
    static ControlSignals decode(const Instruction& inst);
};

#endif // CONTROLUNIT_HPP
