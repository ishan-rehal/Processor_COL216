// #include "../header/Processor.hpp"
// #include <iostream>

// // -------------------------
// // Fetch Stage Implementation
// // -------------------------
// void Processor::fetch() {
//     // In a full implementation, you'll fetch the instruction from memory or a file using the current PC.
//     // For this example, we simulate fetching a dummy I-type instruction: ADDI x1, x0, 5

//     Instruction inst;
//     inst.type = InstType::I_TYPE;
//     inst.opcode = 0x13; // opcode for I-type arithmetic (ADDI)
//     inst.info.i.rd = 1;       // destination register x1
//     inst.info.i.rs1 = 0;      // source register x0
//     inst.info.i.imm = 5;      // immediate value 5
//     inst.info.i.funct3 = 0;   // funct3 for ADDI

//     // Update the IF/ID latch with fetched instruction and current PC.
//     if_id.instruction = inst;
//     if_id.pc = PC;

//     // For a real design, PC might be updated based on branch/jump logic. Here, we simply increment.
//     PC += 4;
// }

// // -------------------------
// // Decode Stage Implementation
// // -------------------------
// void Processor::decode() {
//     // Transfer the fetched instruction and PC to the ID/EX latch.
//     id_ex.pc = if_id.pc;
//     id_ex.instruction = if_id.instruction;

//     // Set control signals based on instruction type. Here we handle only an I-type ADDI as an example.
//     if (if_id.instruction.type == InstType::I_TYPE) {
//         id_ex.regWrite = true;
//         id_ex.memRead  = false;
//         id_ex.memWrite = false;
//         id_ex.branch   = false;
//         id_ex.aluOp    = 0; // Let's assume 0 represents ADD

//         // Read the register file. For ADDI, rs1 is used and immediate is added.
//         id_ex.rs1Val = regs[if_id.instruction.info.i.rs1];
//         id_ex.rs2Val = 0; // Not used for I-type arithmetic
//         id_ex.imm    = if_id.instruction.info.i.imm;
//     }
//     // Extend with decoding for other instruction types as needed.
// }

// // -------------------------
// // Execute Stage Implementation
// // -------------------------
// void Processor::execute() {
//     // Determine the operands.
//     // For ADDI, the second operand is the immediate.
//     uint32_t operand1 = id_ex.rs1Val;
//     uint32_t operand2 = id_ex.imm;
//     uint32_t aluResult = 0;

//     // If forwarding is enabled, you would add logic here to check for data hazards
//     // and substitute forwarded values from later pipeline latches if available.
//     if (forwardingEnabled) {
//         // TODO: Implement forwarding logic.
//     } else {
//         // TODO: Implement hazard detection and stall logic.
//     }

//     // For aluOp = 0, perform addition (i.e., ADDI).
//     if (id_ex.aluOp == 0) {
//         aluResult = operand1 + operand2;
//     }
    
//     // Fill the EX/MEM latch with the ALU result and control signals.
//     ex_mem.aluResult    = aluResult;
//     ex_mem.rs2Val       = id_ex.rs2Val;
//     ex_mem.regWrite     = id_ex.regWrite;
//     ex_mem.memRead      = id_ex.memRead;
//     ex_mem.memWrite     = id_ex.memWrite;
//     ex_mem.branch       = id_ex.branch;
//     ex_mem.instruction  = id_ex.instruction;

//     // For branch instructions, compute branch target.
//     ex_mem.branchTarget = id_ex.pc + id_ex.imm;
// }

// // -------------------------
// // Memory Access Stage Implementation
// // -------------------------
// void Processor::memAccess() {
//     // In a full design, handle load and store instructions.
//     // For our dummy ADDI, there's no memory operation.
//     // Simply transfer the ALU result to the MEM/WB latch.
//     mem_wb.writeData   = ex_mem.aluResult;
//     mem_wb.regWrite    = ex_mem.regWrite;
//     mem_wb.instruction = ex_mem.instruction;
// }

// // -------------------------
// // Write-Back Stage Implementation
// // -------------------------
// void Processor::writeBack() {
//     // If the regWrite signal is set, update the register file.
//     if (mem_wb.regWrite) {
//         uint8_t rd = 0;
//         // Select the appropriate field for the destination register based on the instruction type.
//         switch(mem_wb.instruction.type) {
//             case InstType::I_TYPE:
//                 rd = mem_wb.instruction.info.i.rd;
//                 break;
//             case InstType::R_TYPE:
//                 rd = mem_wb.instruction.info.r.rd;
//                 break;
//             case InstType::U_TYPE:
//                 rd = mem_wb.instruction.info.u.rd;
//                 break;
//             case InstType::J_TYPE:
//                 rd = mem_wb.instruction.info.j.rd;
//                 break;
//             default:
//                 break;
//         }
//         regs[rd] = mem_wb.writeData;
//         std::cout << "WriteBack: Register x" << unsigned(rd)
//                   << " updated to " << regs[rd] << std::endl;
//     }
// }




#include "Processor.hpp"
#include "ControlUnit.hpp"
#include "ALU.hpp"
#include <iostream>

// Constructor: load instruction memory and initialize registers and PC.
Processor::Processor(const std::vector<std::string>& instructionsHex, bool forwarding)
    : PC(0), forwardingEnabled(forwarding)
{
    for (const auto &hex : instructionsHex) {
        Instruction inst(hex);
        instructionMemory.push_back(inst);
    }
    regs.resize(32, 0);  // Initialize 32 registers to 0.
}

// Run one cycle by executing all pipeline stages.
void Processor::runCycle() {
    writeBack();
    memAccess();
    execute();
    decode();
    fetch();
}

// -------------------------
// Fetch Stage
// -------------------------
void Processor::fetch() {
    // Use PC to index into instruction memory (assuming 4-byte instructions).
    if (PC / 4 < instructionMemory.size()) {
        if_id.instruction = instructionMemory[PC / 4];
        if_id.pc = PC;
        PC += 4;
    } else {
        // If no more instructions, inject a NOP.
        Instruction nop;
        nop.type = InstType::NOP;
        if_id.instruction = nop;
        if_id.pc = PC;
    }
}

// -------------------------
// Decode Stage
// -------------------------
void Processor::decode() {
    id_ex.pc = if_id.pc;
    id_ex.instruction = if_id.instruction;

    auto signals = ControlUnit::decode(if_id.instruction);
    id_ex.regWrite = signals.regWrite;
    id_ex.memRead  = signals.memRead;
    id_ex.memWrite = signals.memWrite;
    id_ex.branch   = signals.branch;
    id_ex.aluOp    = signals.aluOp;

    switch (if_id.instruction.type) {
        case InstType::I_TYPE:
            id_ex.rs1Val = regs[if_id.instruction.info.i.rs1];
            id_ex.rs2Val = 0;
            id_ex.imm    = if_id.instruction.info.i.imm;
            break;

        case InstType::R_TYPE:
            id_ex.rs1Val = regs[if_id.instruction.info.r.rs1];
            id_ex.rs2Val = regs[if_id.instruction.info.r.rs2];
            id_ex.imm    = 0;  // No immediate in R-type
            break;

        case InstType::S_TYPE:
            id_ex.rs1Val = regs[if_id.instruction.info.s.rs1];
            id_ex.rs2Val = regs[if_id.instruction.info.s.rs2];
            id_ex.imm    = if_id.instruction.info.s.imm;
            break;

        case InstType::B_TYPE:
            id_ex.rs1Val = regs[if_id.instruction.info.b.rs1];
            id_ex.rs2Val = regs[if_id.instruction.info.b.rs2];
            id_ex.imm    = if_id.instruction.info.b.imm;
            break;

        case InstType::U_TYPE:
            id_ex.rs1Val = 0; // Not used, but safe to clear
            id_ex.rs2Val = 0;
            id_ex.imm    = if_id.instruction.info.u.imm;
            break;

        case InstType::J_TYPE:
            id_ex.rs1Val = 0;
            id_ex.rs2Val = 0;
            id_ex.imm    = if_id.instruction.info.j.imm;
            break;

        default:
            id_ex.rs1Val = 0;
            id_ex.rs2Val = 0;
            id_ex.imm    = 0;
            break;
    }
}


// -------------------------
// Execute Stage
// -------------------------
void Processor::execute() {
    uint32_t operand1 = id_ex.rs1Val;
    uint32_t operand2 = id_ex.imm;  // For I-type arithmetic, immediate is used as the second operand.
    uint32_t aluResult = 0;

    // Forwarding and hazard detection logic.
    if (forwardingEnabled) {
        // TODO: Implement forwarding logic.
    } else {
        // TODO: Implement hazard detection and stall logic.
    }

    // Use a switch-case based on the ALUOp enum.
    switch (id_ex.aluOp) {
        case ALUOp::ADD:
            aluResult = ALU::add(operand1, operand2);
            break;
        case ALUOp::SUB:
            aluResult = ALU::sub(operand1, operand2);
            break;
        case ALUOp::MUL:
            aluResult = ALU::mul(operand1, operand2);
            break;
        case ALUOp::DIV:
            aluResult = ALU::div(operand1, operand2);
            break;
        default:
            aluResult = 0;
            break;
    }
    
    // Populate the EX/MEM latch.
    ex_mem.aluResult    = aluResult;
    ex_mem.rs2Val       = id_ex.rs2Val;
    ex_mem.regWrite     = id_ex.regWrite;
    ex_mem.memRead      = id_ex.memRead;
    ex_mem.memWrite     = id_ex.memWrite;
    ex_mem.branch       = id_ex.branch;
    ex_mem.instruction  = id_ex.instruction;
    ex_mem.branchTarget = id_ex.pc + id_ex.imm;  // For branch instructions.
}


// -------------------------
// Memory Access Stage
// -------------------------
void Processor::memAccess() {
    // For instructions like ADDI there is no memory access.
    // Simply pass the ALU result.
    mem_wb.writeData   = ex_mem.aluResult;
    mem_wb.regWrite    = ex_mem.regWrite;
    mem_wb.instruction = ex_mem.instruction;
}

// -------------------------
// Write-Back Stage
// -------------------------
void Processor::writeBack() {
    if (mem_wb.regWrite) {
        uint8_t rd = 0;
        // Determine the destination register based on the instruction type.
        switch(mem_wb.instruction.type) {
            case InstType::I_TYPE:
                rd = mem_wb.instruction.info.i.rd;
                break;
            case InstType::R_TYPE:
                rd = mem_wb.instruction.info.r.rd;
                break;
            case InstType::U_TYPE:
                rd = mem_wb.instruction.info.u.rd;
                break;
            case InstType::J_TYPE:
                rd = mem_wb.instruction.info.j.rd;
                break;
            default:
                break;
        }
        regs[rd] = mem_wb.writeData;
        std::cout << "WriteBack: Register x" << unsigned(rd)
                  << " updated to " << regs[rd] << std::endl;
    }
}

// -------------------------
// Debug: Print Pipeline State
// -------------------------
void Processor::printPipelineState() {
    std::cout << "----- Pipeline State -----" << std::endl;
    std::cout << "IF/ID - PC: " << if_id.pc 
              << ", Instruction: " << if_id.instruction.rawHex << std::endl;
    std::cout << "ID/EX - PC: " << id_ex.pc << std::endl;
    std::cout << "EX/MEM - ALU Result: " << ex_mem.aluResult << std::endl;
    std::cout << "MEM/WB - Write Data: " << mem_wb.writeData << std::endl;
}
