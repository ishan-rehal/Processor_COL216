#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <vector>
#include <string>
#include "Instruction.hpp"
#include "PipelineStage.hpp"

class Processor {
public:
    uint32_t PC;
    bool forwardingEnabled;
    bool stallIF = false;
    bool stallNeeded = false;
    int totalCycleCount;      // Total number of cycles (from input)
    int currentCycle;         // Simulation cycle counter (full cycles)
    bool headerPrinted;       // To print header only once
    std::vector<std::string> asmInstructions;  // New vector for assembly statements
    std::vector<int> regs;  // 32 general-purpose registers.
    std::vector<Instruction> instructionMemory;

    std::vector<uint8_t> stack_memory;
    
    // Pipeline latches.
    IF_ID_Latch if_id;
    ID_EX_Latch id_ex;
    EX_MEM_Latch ex_mem;
    MEM_WB_Latch mem_wb;
    IF_ID_Latch next_if_id;
    ID_EX_Latch next_id_ex;
    EX_MEM_Latch next_ex_mem;
    MEM_WB_Latch next_mem_wb;

    // Pipeline log: one row per instruction; each row is a vector of strings (one cell per cycle)
    std::vector<std::vector<std::string>> pipelineLog;

    // Constructor: loads instructions from hex strings and sets forwarding mode.
    Processor(const std::vector<std::string>& instructionsHex, bool forwarding, int totalCycleCount,const std::vector<std::string>& asmInstr);
    
    // Resets the processor state.
    uint8_t getRD(const Instruction &inst);
    // Runs one simulation cycle (calls all pipeline stages).
    void runCycle();


    // Helper functions for logging.
    void logInstructionStage(const Instruction &instr, const std::string &stage);
    void printPipelineLogHeader() const;
    void printInstructionLog(int instrId) const;




    // Pipeline stage functions.
    void fetch(int cycle);
    void decode(int cycle);
    void execute(int cycle);
    void memAccess(int cycle);
    void writeBack(int cycle);
    void updateLatches();

    // Helper function: prints the current state of the pipeline.
    void printPipelineState();
    void debug_print();
    void print_registers();
    void printFullPipelineLog() const;
    void printFullPipelineLogSimple() const;
};

#endif // PROCESSOR_HPP
