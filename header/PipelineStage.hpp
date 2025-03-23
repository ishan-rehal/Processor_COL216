#ifndef PIPELINESTAGE_HPP
#define PIPELINESTAGE_HPP

#include "Instruction.hpp"
#include <cstdint>

// Latch between Instruction Fetch and Decode stages.
struct IF_ID_Latch {
    Instruction instruction;
    uint32_t pc;
};

// Latch between Decode and Execute stages.
struct ID_EX_Latch {
    uint32_t pc;
    Instruction instruction;
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    int aluOp;
    uint32_t rs1Val;
    uint32_t rs2Val;
    int imm;
};

// Latch between Execute and Memory stages.
struct EX_MEM_Latch {
    uint32_t aluResult;
    uint32_t rs2Val;
    bool regWrite;
    bool memRead;
    bool memWrite;
    bool branch;
    uint32_t branchTarget;
    Instruction instruction;
};

// Latch between Memory and Write-Back stages.
struct MEM_WB_Latch {
    uint32_t writeData;
    bool regWrite;
    Instruction instruction;
};

#endif // PIPELINESTAGE_HPP
