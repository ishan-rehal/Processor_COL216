#ifndef PIPELINESTAGE_HPP
#define PIPELINESTAGE_HPP

#include "Instruction.hpp"
#include "ControlUnit.hpp" // This now includes the definition for ALUOp

struct IF_ID_Latch {
    Instruction instruction;
    uint32_t pc;
};

struct ID_EX_Latch {
    uint32_t pc;
    Instruction instruction;
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    ALUOp aluOp;      // Change type from int to ALUOp
    uint32_t rs1Val;
    uint32_t rs2Val;
    int imm;
};

struct EX_MEM_Latch {
    int aluResult;
    uint32_t rs2Val;
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    uint32_t branchTarget;
    Instruction instruction;
};

struct MEM_WB_Latch {
    int writeData = 0;
    bool regWrite;
    Instruction instruction;
};

#endif // PIPELINESTAGE_HPP
