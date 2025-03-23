#include "ControlUnit.hpp"

ControlSignals ControlUnit::decode(const Instruction &inst) {
    ControlSignals signals = {false, false, false, false, ALUOp::NONE};

    // R-type instructions (opcode 0x33)
    if (inst.type == InstType::R_TYPE) {
        signals.regWrite = true;
        // Example: if funct3 is 0 and funct7 is 0x00 => ADD,
        // if funct7 is 0x20 => SUB, and if funct7 is 0x01 (M-extension) then check funct3 for MUL/DIV.
        if (inst.funct3 == 0x0) {
            if (inst.funct7 == 0x00) {
                signals.aluOp = ALUOp::ADD;
            } else if (inst.funct7 == 0x20) {
                signals.aluOp = ALUOp::SUB;
            } else if (inst.funct7 == 0x01) {
                if (inst.funct3 == 0x0) {
                    signals.aluOp = ALUOp::MUL;
                } else if (inst.funct3 == 0x4) {
                    signals.aluOp = ALUOp::DIV;
                }
            }
        }
    }
    // I-type instructions (opcode 0x13 for ADDI, or 0x03 for LOAD)
    else if (inst.type == InstType::I_TYPE) {
        signals.regWrite = true;
        if (inst.opcode == 0x13) {  // ADDI
            signals.aluOp = ALUOp::ADD;
        } else if (inst.opcode == 0x03) {  // LOAD
            signals.memRead = true;
            signals.aluOp = ALUOp::ADD;  // For effective address computation.
        }
    }
    // S-type instructions (opcode 0x23 for STORE)
    else if (inst.opcode == 0x23) {  // S-type
        signals.regWrite = false;
        signals.memWrite = true;
        signals.aluOp = ALUOp::ADD;  // Compute effective address: rs1 + immediate.
    }
    // B-type instructions (opcode 0x63 for Branch)
    else if (inst.opcode == 0x63) {  // B-type
        signals.regWrite = false;
        signals.memRead = false;
        signals.memWrite = false;
        signals.branch = true;
        // Often, branch instructions compare two registers.
        // Here we set the ALU operation to SUB so that the result (or a flag) can be used to determine if the branch should be taken.
        signals.aluOp = ALUOp::SUB;
    }
    // U-type and J-type can be extended similarly.
    else if (inst.type == InstType::U_TYPE) {
        signals.regWrite = true;
        signals.aluOp = ALUOp::NONE;
    }
    else if (inst.type == InstType::J_TYPE) {
        signals.regWrite = true;
        signals.aluOp = ALUOp::NONE;
    }
    
    return signals;
}
