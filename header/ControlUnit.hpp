#ifndef CONTROLUNIT_HPP
#define CONTROLUNIT_HPP

#include "Instruction.hpp"


// Define ALU operation types.
enum class ALUOp {
    NONE,
    ADD,
    SUB,
    MUL,
    DIV
    // You can add more (e.g., AND, OR, etc.) as needed.
};


// Structure holding the control signals for an instruction.
struct ControlSignals {
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    ALUOp aluOp; 
};

class ControlUnit {
public:
    // Decodes the instruction and returns its control signals.
    static ControlSignals decode(const Instruction& inst);
};

#endif // CONTROLUNIT_HPP
