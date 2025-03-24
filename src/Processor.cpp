// Processor.cpp
#include "Processor.hpp"
#include "ControlUnit.hpp"
#include "ALU.hpp"
#include <iostream>

// Constructor: load instruction memory, initialize registers, PC, and pipeline latches.
Processor::Processor(const std::vector<std::string>& instructionsHex, bool forwarding)
    : PC(0), forwardingEnabled(forwarding)
{
    // Load instructions from hex strings.
    for (const auto &hex : instructionsHex) {
        Instruction inst(hex);
        instructionMemory.push_back(inst);
    }
    regs.resize(32, 0);  // Initialize 32 registers to 0.
    
    // Initialize pipeline registers to NOP.
    Instruction nop;
    nop.type = InstType::NOP;
    if_id.instruction = nop;
    id_ex.instruction = nop;
    ex_mem.instruction = nop;
    mem_wb.instruction = nop;
    
    // Also initialize the temporary "next" latches.
    next_if_id = if_id;
    next_id_ex = id_ex;
    next_ex_mem = ex_mem;
    next_mem_wb = mem_wb;
}

// -------------------------
// Fetch Stage (with cycle parameter)
// -------------------------
void Processor::fetch(int cycle) {
    if (cycle == 0) {
        // First half: fetch an instruction based on the current PC.
        if (PC / 4 < instructionMemory.size()) {
            next_if_id.instruction = instructionMemory[PC / 4];
            next_if_id.pc = PC;
        } else {
            // If no more instructions, inject a NOP.
            Instruction nop;
            nop.type = InstType::NOP;
            next_if_id.instruction = nop;
            next_if_id.pc = PC;
        }
    } else {
        // Second half: no additional work for fetch.
    }
}

void Processor::decode(int cycle) {
    if (cycle == 1) {  

        print_registers();

        // Second half: perform complete decode work.
        std::cout << "Decoding instruction: "<< std::endl;
        if_id.instruction.printc_instruction();
        std::cout << std::endl;
        // 1. Decode the instruction (if not a NOP).
        if (if_id.instruction.type != InstType::NOP)
            if_id.instruction.decode();
        
        // 2. Copy PC and instruction from IF/ID to next ID/EX latch.
        next_id_ex.pc = if_id.pc;
        next_id_ex.instruction = if_id.instruction;
        
        // 3. Generate control signals based on the instruction.
        auto signals = ControlUnit::decode(if_id.instruction);
        next_id_ex.regWrite = signals.regWrite;
        next_id_ex.memRead  = signals.memRead;
        next_id_ex.memWrite = signals.memWrite;
        next_id_ex.branch   = signals.branch;
        next_id_ex.aluOp    = signals.aluOp;
        
        // 4. Read the register file using the instruction in next_id_ex.
        switch (next_id_ex.instruction.type) {
            case InstType::I_TYPE:
                next_id_ex.rs1Val = regs[next_id_ex.instruction.info.i.rs1];
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = next_id_ex.instruction.info.i.imm;
                break;
            case InstType::R_TYPE:
                next_id_ex.rs1Val = regs[next_id_ex.instruction.info.r.rs1];
                next_id_ex.rs2Val = regs[next_id_ex.instruction.info.r.rs2];
                next_id_ex.imm    = 0; // R-type instructions have no immediate.
                break;
            case InstType::S_TYPE:
                next_id_ex.rs1Val = regs[next_id_ex.instruction.info.s.rs1];
                next_id_ex.rs2Val = regs[next_id_ex.instruction.info.s.rs2];
                next_id_ex.imm    = next_id_ex.instruction.info.s.imm;
                break;
            case InstType::B_TYPE:
                next_id_ex.rs1Val = regs[next_id_ex.instruction.info.b.rs1];
                next_id_ex.rs2Val = regs[next_id_ex.instruction.info.b.rs2];
                next_id_ex.imm    = next_id_ex.instruction.info.b.imm;
                break;
            case InstType::U_TYPE:
                next_id_ex.rs1Val = 0;
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = next_id_ex.instruction.info.u.imm;
                break;
            case InstType::J_TYPE:
                next_id_ex.rs1Val = 0;
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = next_id_ex.instruction.info.j.imm;
                break;
            default:
                next_id_ex.rs1Val = 0;
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = 0;
                break;
        }
    }
    // If cycle == 0, do nothing.
}



// -------------------------
// Execute Stage (with cycle parameter)
// -------------------------
void Processor::execute(int cycle) {
    if (cycle == 0) {
        uint32_t operand1 = id_ex.rs1Val;
        uint32_t operand2 = 0;

        // Set operand2 based on instruction type.
        switch (id_ex.instruction.type) {
            case InstType::R_TYPE:
                operand2 = id_ex.rs2Val;
                break;
            case InstType::I_TYPE:
                operand2 = id_ex.imm;
                break;
            case InstType::S_TYPE:
                operand2 = id_ex.imm;
                break;
            case InstType::B_TYPE:
                operand2 = id_ex.rs2Val;
                break;
            case InstType::U_TYPE:
                // For U-type, operand2 may not be used by the ALU.
                operand2 = id_ex.imm;
                break;
            case InstType::J_TYPE: // ye PC update mei krna h
                // For J-type (e.g., JAL), we compute the jump target.
                // The jump target is computed as current PC + immediate.
                // Although we set operand2 here, the main purpose is to compute branchTarget.
                operand2 = id_ex.imm;
                break;
            default:
                operand2 = id_ex.imm;
                break;
        }

        uint32_t aluResult = 0;
        // Perform the ALU operation as needed.
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
        
        // For branch or jump instructions (B-type and J-type),
        // compute the branch/jump target address.
        next_ex_mem.branchTarget = id_ex.pc + id_ex.imm;
        
        // Prepare next EX/MEM latch.
        next_ex_mem.aluResult = aluResult;
        next_ex_mem.rs2Val = id_ex.rs2Val;
        next_ex_mem.regWrite = id_ex.regWrite;
        next_ex_mem.memRead = id_ex.memRead;
        next_ex_mem.memWrite = id_ex.memWrite;
        next_ex_mem.branch = id_ex.branch;
        next_ex_mem.instruction = id_ex.instruction;

    }
    // Second half: no additional work in execute stage.
}



// -------------------------
// Memory Access Stage (with cycle parameter)
// -------------------------
void Processor::memAccess(int cycle) {
    if (cycle == 0) {
        // For an instruction like ADDI there is no memory access.
        // Simply pass the ALU result forward.
        next_mem_wb.writeData = ex_mem.aluResult;
        next_mem_wb.regWrite = ex_mem.regWrite;
        next_mem_wb.instruction = ex_mem.instruction;
    } else {
        // Second half: no additional work in memory access stage.
    }
}

// -------------------------
// Write-Back Stage (with cycle parameter)
// -------------------------
void Processor::writeBack(int cycle) {
    if (cycle == 0) {  // First half: perform write-back.
        if (mem_wb.regWrite) {
            uint8_t rd = 0;
            switch (mem_wb.instruction.type) {
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
    } else {
        // Second half: no write operations.
    }
}


// -------------------------
// Update Pipeline Latches and PC
// -------------------------
void Processor::updateLatches() {
    // Commit the computed next states to the current pipeline registers.
    if_id = next_if_id;
    id_ex = next_id_ex;
    ex_mem = next_ex_mem;
    mem_wb = next_mem_wb;
    
    // Update the PC (assume no branch for now).
    PC += 4;
}

// -------------------------
// Run One Full Cycle
// -------------------------
void Processor::runCycle() {
    // First half (cycle = 0) for all stages.
    fetch(0);
    decode(0);
    execute(0);
    memAccess(0);
    writeBack(0); // Write-back is done in the first half.
    std::cout << "First half cycle over for" << PC / 4 << ":\n";
    print_registers();
    // Second half (cycle = 1) for all stages.
    fetch(1);
    decode(1);
    execute(1);
    memAccess(1);
    writeBack(1);
    
    // Commit the computed next state and update the PC.
    updateLatches();
    
    // Print the concise pipeline state.
    printPipelineState();
    
    // Also print detailed pipeline debug info.
    // debug_print();
}

void Processor::debug_print() {
    std::cout << "----- Debug Print: Pipeline Latches -----" << std::endl;
    
    std::cout << "IF/ID Stage: Instruction: ";
    if_id.instruction.printc_instruction();
    std::cout << std::endl << ", PC: " << if_id.pc << std::endl;
    
    
    std::cout << "ID/EX Stage: Instruction: ";
    id_ex.instruction.printc_instruction();
    std::cout << ", PC: " << id_ex.pc;
    std::cout << ", rs1Val: " << id_ex.rs1Val 
              << ", rs2Val: " << id_ex.rs2Val 
              << ", Imm: " << id_ex.imm << std::endl;
    
    std::cout << "EX/MEM Stage: Instruction: ";
    ex_mem.instruction.printc_instruction();
    std::cout << ", ALU Result: " << ex_mem.aluResult
              << ", rs2Val: " << ex_mem.rs2Val << std::endl;
    
    std::cout << "MEM/WB Stage: Instruction: ";
    mem_wb.instruction.printc_instruction();
    std::cout << ", Write Data: " << mem_wb.writeData << std::endl;
    
    std::cout << "-------------------------------------------" << std::endl;
}


// -------------------------
// Debug: Print Pipeline State
// -------------------------
void Processor::printPipelineState() {
    std::cout << "----- Pipeline State -----" << std::endl;
    std::cout << "IF/ID: "<< std::endl;
    if_id.instruction.printc_instruction();
    std::cout << std::endl;
    
    std::cout << "ID/EX: "<< std::endl;
    id_ex.instruction.printc_instruction();
    std::cout << std::endl;
    
    std::cout << "EX/MEM: "<< std::endl;
    ex_mem.instruction.printc_instruction();
    std::cout << std::endl;
    std::cout << " | ALU Result: " << ex_mem.aluResult << std::endl;
    
    std::cout << "MEM/WB: "<< std::endl;
    mem_wb.instruction.printc_instruction();
    std::cout << std::endl;
    std::cout << " | Write Data: " << mem_wb.writeData << std::endl;
}

void Processor::print_registers()
{
    std::cout << "Registers: " << std::endl;
    for (int i = 0; i < 32; i++)
    {
        std::cout << "x" << i << ": " << regs[i] << std::endl;
    }

}