// Processor.cpp
#include "Processor.hpp"
#include "ControlUnit.hpp"
#include "ALU.hpp"
#include <iostream>

// Constructor: initialize registers, PC, pipeline latches, and the stack memory.
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
    
    next_if_id = if_id;
    next_id_ex = id_ex;
    next_ex_mem = ex_mem;
    next_mem_wb = mem_wb;
    
    // NEW: Initialize stack memory to 1024 bytes (all zeros)
    stack_memory.resize(1024, 0);
}


// -------------------------
// Fetch Stage (with cycle parameter)
// -------------------------
void Processor::fetch(int cycle) {
    if (cycle == 0) {
        // If decode stage inserted a NOP, do nothing
        if (next_if_id.instruction.type == InstType::NOP) {
            next_if_id.pc = PC;
        } 
        else if (PC / 4 < instructionMemory.size()) {
            next_if_id.instruction = instructionMemory[PC / 4];
            next_if_id.pc = PC;
        } 
        else {
            Instruction nop;
            nop.type = InstType::NOP;
            next_if_id.instruction = nop;
            next_if_id.pc = PC;
        }
    }
}



void Processor::decode(int cycle) {
    if (cycle == 1) {
        // Print registers for debugging, optional
        print_registers();

        // Print which instruction is being decoded
        std::cout << "Decoding instruction: ";
        if_id.instruction.printc_instruction();
        std::cout << std::endl;

        // Decode the raw fields if it's not a NOP
        if (if_id.instruction.type != InstType::NOP) {
            if_id.instruction.decode();
        }

        // Get control signals
        auto signals = ControlUnit::decode(if_id.instruction);

        // Copy PC, instruction, and signals into ID/EX
        next_id_ex.pc         = if_id.pc;
        next_id_ex.instruction = if_id.instruction;
        next_id_ex.regWrite   = signals.regWrite;
        next_id_ex.memRead    = signals.memRead;
        next_id_ex.memWrite   = signals.memWrite;
        next_id_ex.branch     = signals.branch;
        next_id_ex.aluOp      = signals.aluOp;

        // Read registers and immediate depending on instruction type
        switch (next_id_ex.instruction.type) {
          case InstType::I_TYPE:
            next_id_ex.rs1Val = regs[next_id_ex.instruction.info.i.rs1];
            next_id_ex.rs2Val = 0;
            next_id_ex.imm    = next_id_ex.instruction.info.i.imm;
            break;

          case InstType::R_TYPE:
            next_id_ex.rs1Val = regs[next_id_ex.instruction.info.r.rs1];
            next_id_ex.rs2Val = regs[next_id_ex.instruction.info.r.rs2];
            next_id_ex.imm    = 0;
            break;

          case InstType::S_TYPE:
            next_id_ex.rs1Val = regs[next_id_ex.instruction.info.s.rs1];
            next_id_ex.rs2Val = regs[next_id_ex.instruction.info.s.rs2];
            next_id_ex.imm    = next_id_ex.instruction.info.s.imm;
            break;

          case InstType::B_TYPE: {
            // Read registers
            uint32_t rs1Val = regs[next_id_ex.instruction.info.b.rs1];
            uint32_t rs2Val = regs[next_id_ex.instruction.info.b.rs2];

            // Sign-extend immediate as needed
            int32_t offset = next_id_ex.instruction.info.b.imm; // offset for the branch

            // Determine which branch type via funct3
            uint8_t f3 = next_id_ex.instruction.info.b.funct3;
            bool branchTaken = false;

            switch (f3) {
                case 0x0: // BEQ
                    branchTaken = (rs1Val == rs2Val);
                    break;
                case 0x1: // BNE
                    branchTaken = (rs1Val != rs2Val);
                    break;
                case 0x4: // BLT (signed)
                    branchTaken = (static_cast<int32_t>(rs1Val) 
                                   < static_cast<int32_t>(rs2Val));
                    break;
                case 0x5: // BGE (signed)
                    branchTaken = (static_cast<int32_t>(rs1Val) 
                                   >= static_cast<int32_t>(rs2Val));
                    break;
                case 0x6: // BLTU (unsigned)
                    branchTaken = (rs1Val < rs2Val);
                    break;
                case 0x7: // BGEU (unsigned)
                    branchTaken = (rs1Val >= rs2Val);
                    break;
                default:
                    // Handle other potential encodings or unknown branch types
                    std::cerr << "Unknown B-type funct3: " << (int)f3 << std::endl;
                    break;
            }

            // Update PC based on branch decision
            if (branchTaken) {
                PC = next_id_ex.pc + offset; // Jump to the branch target
            } else {
                PC = next_id_ex.pc + 4;      // Continue sequentially
            }

            // Insert a NOP into IF for next cycle
            Instruction nop;
            nop.type = InstType::NOP;
            next_if_id.instruction = nop;
            next_if_id.pc = PC;  // Or just keep the same PC if you prefer

            // Pass forward something for EX, though won't do real ALU ops for a branch
            next_id_ex.rs1Val = rs1Val;
            next_id_ex.rs2Val = rs2Val;
            next_id_ex.imm    = offset;

            break;
          }

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
}



// -------------------------
// Execute Stage (with cycle parameter)
// -------------------------
void Processor::execute(int cycle) {
    if (cycle == 0) {
        uint32_t operand1 = id_ex.rs1Val;
        uint32_t operand2 = 0;
        
        // For I-type instructions, use immediate.
        // For R-type instructions, use rs2Val.
        if (id_ex.instruction.type == InstType::I_TYPE) {
            operand2 = id_ex.imm;
        } else if (id_ex.instruction.type == InstType::R_TYPE) {
            operand2 = id_ex.rs2Val;
        } else {
            // For other types, adjust accordingly.
            operand2 = id_ex.imm;
        }
        
        uint32_t aluResult = 0;
        
        // Compute ALU operation based on aluOp.
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
        
        // Prepare next EX/MEM latch.
        next_ex_mem.aluResult = aluResult;
        next_ex_mem.rs2Val = id_ex.rs2Val;
        next_ex_mem.regWrite = id_ex.regWrite;
        next_ex_mem.memRead = id_ex.memRead;
        next_ex_mem.memWrite = id_ex.memWrite;
        next_ex_mem.branch = id_ex.branch;
        next_ex_mem.instruction = id_ex.instruction;
        next_ex_mem.branchTarget = id_ex.pc + id_ex.imm;
    } else {
        // Second half: no additional work in execute stage.
    }
}


// -------------------------
// Memory Access Stage (with cycle parameter)
// -------------------------
void Processor::memAccess(int cycle) {
    // Perform the memory operation in the whole cycle.
    uint32_t addr = ex_mem.aluResult;
    
    // If this is a store operation, perform it.
    if (ex_mem.memWrite) {
        uint32_t value = ex_mem.rs2Val;
        uint8_t funct3 = ex_mem.instruction.info.s.funct3;
        switch (funct3) {
            case 0: // SB: Store Byte
                if (addr < stack_memory.size())
                    stack_memory[addr] = value & 0xFF;
                break;
            case 1: // SH: Store Halfword
                if (addr + 1 < stack_memory.size()) {
                    stack_memory[addr] = value & 0xFF;
                    stack_memory[addr + 1] = (value >> 8) & 0xFF;
                }
                break;
            case 2: // SW: Store Word
                if (addr + 3 < stack_memory.size()) {
                    stack_memory[addr]     = value & 0xFF;
                    stack_memory[addr + 1] = (value >> 8) & 0xFF;
                    stack_memory[addr + 2] = (value >> 16) & 0xFF;
                    stack_memory[addr + 3] = (value >> 24) & 0xFF;
                }
                break;
            case 3: // SD: Store Doubleword (if supported)
                if (addr + 7 < stack_memory.size()) {
                    for (int i = 0; i < 8; i++) {
                        stack_memory[addr + i] = (value >> (8 * i)) & 0xFF;
                    }
                }
                break;
            default:
                // Unsupported store type.
                break;
        }
        // For stores, no value is forwarded for write-back.
        Instruction nop;
        nop.type = InstType::NOP;
        next_mem_wb.instruction = nop;
        next_mem_wb.regWrite = false;
    }
    // Else if this is a load operation, perform it.
    else if (ex_mem.memRead) {
        uint32_t data = 0;
        uint8_t funct3 = ex_mem.instruction.info.i.funct3;
        switch (funct3) {
            case 0: { // LB: Load Byte (sign-extended)
                if (addr < stack_memory.size()) {
                    int8_t byte = static_cast<int8_t>(stack_memory[addr]);
                    data = static_cast<int32_t>(byte);
                }
                break;
            }
            case 4: { // LBU: Load Byte Unsigned
                if (addr < stack_memory.size()) {
                    data = stack_memory[addr];
                }
                break;
            }
            case 1: { // LH: Load Halfword (sign-extended)
                if (addr + 1 < stack_memory.size()) {
                    int16_t half = static_cast<int16_t>(
                        stack_memory[addr] | (stack_memory[addr + 1] << 8)
                    );
                    data = static_cast<int32_t>(half);
                }
                break;
            }
            case 5: { // LHU: Load Halfword Unsigned
                if (addr + 1 < stack_memory.size()) {
                    data = stack_memory[addr] | (stack_memory[addr + 1] << 8);
                }
                break;
            }
            case 2: { // LW: Load Word
                if (addr + 3 < stack_memory.size()) {
                    data = stack_memory[addr] |
                           (stack_memory[addr + 1] << 8) |
                           (stack_memory[addr + 2] << 16) |
                           (stack_memory[addr + 3] << 24);
                }
                break;
            }
            case 6: { // LWU: Load Word Unsigned
                if (addr + 3 < stack_memory.size()) {
                    data = stack_memory[addr] |
                           (stack_memory[addr + 1] << 8) |
                           (stack_memory[addr + 2] << 16) |
                           (stack_memory[addr + 3] << 24);
                }
                break;
            }
            // You can add support for LD (doubleword load) if desired.
            default:
                // Unsupported load type.
                break;
        }
        next_mem_wb.writeData = data;
        next_mem_wb.regWrite = ex_mem.regWrite;
        next_mem_wb.instruction = ex_mem.instruction;
    }
    // If no memory operation is required, simply forward the ALU result.
    else {
        next_mem_wb.writeData = ex_mem.aluResult;
        next_mem_wb.regWrite = ex_mem.regWrite;
        next_mem_wb.instruction = ex_mem.instruction;
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
    if_id = next_if_id;
    id_ex = next_id_ex;
    ex_mem = next_ex_mem;
    mem_wb = next_mem_wb;

    // If the instruction wasn't a branch, then increment normally
    // If it was a branch, we've already updated PC in decode.
    if (id_ex.instruction.type != InstType::B_TYPE) {
        PC += 4;
    }
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

    // Second half (cycle = 1) for all stages.
    fetch(1);
    decode(1);
    execute(1);
    // memAccess(1); // Memory access is processed through the whole cycle.
    // writeBack(1); // Write-back is done in the first half.
    
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