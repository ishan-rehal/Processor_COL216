// #ifndef PROCESSOR_HPP
// #define PROCESSOR_HPP

// #include <cstdint>
// #include <array>

// // --------------------------------------------------------
// // 1) Define the instruction types
// // --------------------------------------------------------
// enum class InstType {
//     R_TYPE,
//     I_TYPE,
//     S_TYPE,
//     B_TYPE,
//     U_TYPE,
//     J_TYPE,
//     UNKNOWN
// };

// // --------------------------------------------------------
// // 2) Define the Instruction struct
// //    - 'type' indicates which union member is valid
// //    - 'opcode' is common for all instructions
// //    - The union holds the fields relevant to each instruction type
// // --------------------------------------------------------
// struct Instruction {
//     InstType type     = InstType::UNKNOWN;
//     uint8_t  opcode   = 0;   // Common opcode field for all

//     union {
//         // R-type fields
//         struct {
//             uint8_t rd;
//             uint8_t rs1;
//             uint8_t rs2;
//             uint8_t funct3;
//             uint8_t funct7;
//         } r;

//         // I-type fields
//         struct {
//             uint8_t rd;
//             uint8_t rs1;
//             int32_t imm;      // Sign-extended immediate
//             uint8_t funct3;
//         } i;

//         // S-type fields
//         struct {
//             uint8_t rs1;
//             uint8_t rs2;
//             int32_t imm;      // Sign-extended immediate
//             uint8_t funct3;
//         } s;

//         // B-type fields
//         struct {
//             uint8_t rs1;
//             uint8_t rs2;
//             int32_t imm;      // Branch immediate
//             uint8_t funct3;
//         } b;

//         // U-type fields
//         struct {
//             uint8_t rd;
//             int32_t imm;      // Upper immediate
//         } u;

//         // J-type fields
//         struct {
//             uint8_t rd;
//             int32_t imm;      // Jump immediate
//         } j;
//     } info;
// };

// // --------------------------------------------------------
// // 3) Define the 4 pipeline latch structs
// // --------------------------------------------------------

// // IF/ID latch: holds the PC and the raw fetched instruction
// struct IF_ID_Latch {
//     uint32_t    pc           = 0;
//     Instruction instruction; 
// };

// // ID/EX latch: holds decoded instruction info, register values, and control signals
// struct ID_EX_Latch {
//     uint32_t    pc           = 0;
//     Instruction instruction; 

//     // Control signals (expand as needed)
//     bool        regWrite     = false;
//     bool        memRead      = false;
//     bool        memWrite     = false;
//     bool        branch       = false;
//     // Example ALU operation code; you may define an enum instead of uint8_t
//     uint8_t     aluOp        = 0;

//     // Register values and immediate data
//     uint32_t    rs1Val       = 0;
//     uint32_t    rs2Val       = 0;
//     int32_t     imm          = 0;  // Immediate value (I, S, B, J, or U)
// };

// // EX/MEM latch: holds ALU results, branch targets, and control signals for memory stage
// struct EX_MEM_Latch {
//     bool        regWrite     = false;
//     bool        memRead      = false;
//     bool        memWrite     = false;
//     bool        branch       = false;

//     uint32_t    aluResult    = 0;    // Result from the ALU
//     uint32_t    branchTarget = 0;    // Calculated branch target if needed
//     uint32_t    rs2Val       = 0;    // Register value for store operations

//     Instruction instruction;
// };

// // MEM/WB latch: final data (from memory or ALU) and control signals for write-back
// struct MEM_WB_Latch {
//     bool        regWrite     = false;
//     uint32_t    writeData    = 0;    // Data to be written back to the register file

//     Instruction instruction;
// };

// // --------------------------------------------------------
// // 4) Processor class: supports both forwarding and non-forwarding modes
// // --------------------------------------------------------
// class Processor {
// public:
//     // Constructor: pass true to enable forwarding, false for no forwarding.
//     Processor(bool enableForwarding) : forwardingEnabled(enableForwarding) {
//         reset();
//     }
//     ~Processor() = default;

//     // Reset PC, registers, and all pipeline latches.
//     void reset() {
//         PC = 0;
//         regs.fill(0);

//         if_id  = {};
//         id_ex  = {};
//         ex_mem = {};
//         mem_wb = {};
//     }

//     // Run one pipeline cycle (stages executed in reverse order to prevent latch overwrites)
//     void runCycle() {
//         writeBack();
//         memAccess();
//         execute();
//         decode();
//         fetch();
//     }

// private:
//     // Flag to control forwarding vs. non-forwarding functionality.
//     bool forwardingEnabled;

//     // Program Counter (PC)
//     uint32_t PC = 0;

//     // Register file (32 registers)
//     std::array<uint32_t, 32> regs{};

//     // Pipeline latches for each stage boundary
//     IF_ID_Latch  if_id;
//     ID_EX_Latch  id_ex;
//     EX_MEM_Latch ex_mem;
//     MEM_WB_Latch mem_wb;

//     // ---------------------------
//     // Pipeline Stage Methods
//     // ---------------------------
//     // Instruction Fetch (IF) stage
//     void fetch() {
//         // TODO: Read the instruction using PC from instruction memory or a file.
//         // For now, this is a placeholder. Example:
//         // if_id.instruction = <fetched instruction>;
//         // if_id.pc = PC;
//         // Increment PC by 4 (assuming 32-bit instructions).
//         // PC += 4;
//     }

//     // Instruction Decode (ID) stage
//     void decode() {
//         // TODO: Decode if_id.instruction.
//         // Set control signals, read register values into id_ex.
//         // For example:
//         // id_ex.rs1Val = regs[<rs1 extracted from if_id.instruction>];
//         // id_ex.rs2Val = regs[<rs2 extracted from if_id.instruction>];
//         // id_ex.imm    = <sign-extended immediate from if_id.instruction>;
//         // Copy the instruction and PC to id_ex.
//         // id_ex.instruction = if_id.instruction;
//         // id_ex.pc = if_id.pc;
//     }

//     // Execute (EX) stage
//     void execute() {
//         // Depending on the processor mode, execute differently.
//         if (forwardingEnabled) {
//             // 5-Stage pipeline with forwarding:
//             // TODO: Implement forwarding logic here.
//             // Check if the required data is available in the EX/MEM or MEM/WB latches,
//             // and forward the values to the ALU input as necessary.
//             // Example placeholder:
//             // uint32_t operand1 = id_ex.rs1Val;
//             // uint32_t operand2 = id_ex.rs2Val;
//             // // Forwarding: if the source register matches destination in a later stage,
//             // // update operand1/operand2 accordingly.
//             // ex_mem.aluResult = operand1 + operand2; // Example ALU operation based on aluOp.
//         } else {
//             // 5-Stage pipeline without forwarding:
//             // TODO: Insert hazard detection and stalling logic.
//             // If a data hazard is detected, insert a bubble (stall) by delaying the pipeline.
//             // Otherwise, proceed with the ALU operation normally.
//             // Example placeholder:
//             // uint32_t operand1 = id_ex.rs1Val;
//             // uint32_t operand2 = id_ex.rs2Val;
//             // ex_mem.aluResult = operand1 + operand2; // Example ALU operation.
//         }

//         // Set control signals and transfer the instruction to the EX/MEM latch.
//         ex_mem.regWrite     = id_ex.regWrite;
//         ex_mem.memRead      = id_ex.memRead;
//         ex_mem.memWrite     = id_ex.memWrite;
//         ex_mem.branch       = id_ex.branch;
//         ex_mem.rs2Val       = id_ex.rs2Val;
//         ex_mem.instruction  = id_ex.instruction;

//         // For branch target calculations, you may set:
//         // ex_mem.branchTarget = id_ex.pc + id_ex.imm;
//     }

//     // Memory Access (MEM) stage
//     void memAccess() {
//         // TODO: For load instructions, read memory using ex_mem.aluResult.
//         // For store instructions, write ex_mem.rs2Val into memory at ex_mem.aluResult.
//         // Set mem_wb.writeData to the loaded data or pass through the ALU result.
//         // Transfer control signals and the instruction to the MEM/WB latch.
//         mem_wb.regWrite    = ex_mem.regWrite;
//         // Example: If load then mem_wb.writeData = <data from memory>;
//         // Otherwise, mem_wb.writeData = ex_mem.aluResult;
//         mem_wb.writeData   = ex_mem.aluResult; // Placeholder
//         mem_wb.instruction = ex_mem.instruction;
//     }

//     // Write-Back (WB) stage
//     void writeBack() {
//         // TODO: If mem_wb.regWrite is true, write mem_wb.writeData back to the register file.
//         // For example, if the destination register is stored in the instruction's union,
//         // select the proper field based on the instruction type.
//         // Example placeholder:
//         // if (mem_wb.regWrite) {
//         //     uint8_t rd;
//         //     if (mem_wb.instruction.type == InstType::R_TYPE) {
//         //         rd = mem_wb.instruction.info.r.rd;
//         //     }
//         //     else if (mem_wb.instruction.type == InstType::I_TYPE) {
//         //         rd = mem_wb.instruction.info.i.rd;
//         //     }
//         //     // ... (handle other types as needed)
//         //     regs[rd] = mem_wb.writeData;
//         // }
//     }
// };

// #endif // PROCESSOR_HPP




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
    std::vector<uint32_t> regs;  // 32 general-purpose registers.
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

    // Constructor: loads instructions from hex strings and sets forwarding mode.
    Processor(const std::vector<std::string>& instructionsHex, bool forwarding);

    // Runs one simulation cycle (calls all pipeline stages).
    void runCycle();

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
};

#endif // PROCESSOR_HPP
