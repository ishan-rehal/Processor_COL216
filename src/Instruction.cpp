#include "Instruction.hpp"
#include <cstdlib>

Instruction::Instruction(const std::string &hex)
    : rawHex(hex), rawOpcode(0), opcode(0), rd(0), rs1(0), rs2(0),
      funct3(0), funct7(0), immediate(0), type(InstType::UNKNOWN) {
    // Convert the hex string to a 32-bit unsigned integer.
    rawOpcode = std::stoul(hex, nullptr, 16);
    decode();
}

void Instruction::decode() {
    // Extract the opcode (lowest 7 bits).
    opcode = rawOpcode & 0x7F;
    
    // Determine instruction type and decode accordingly.
    // This is a simplified example.
    if (opcode == 0x33) { // R-type example.
        type = InstType::R_TYPE;
        info.r.rd     = (rawOpcode >> 7)  & 0x1F;
        info.r.funct3 = (rawOpcode >> 12) & 0x7;
        info.r.rs1    = (rawOpcode >> 15) & 0x1F;
        info.r.rs2    = (rawOpcode >> 20) & 0x1F;
        info.r.funct7 = (rawOpcode >> 25) & 0x7F;
    }
    else if (opcode == 0x13 || opcode == 0x03) { // I-type example (ADDI, load).
        type = InstType::I_TYPE;
        info.i.rd     = (rawOpcode >> 7)  & 0x1F;
        info.i.funct3 = (rawOpcode >> 12) & 0x7;
        info.i.rs1    = (rawOpcode >> 15) & 0x1F;
        info.i.imm    = static_cast<int32_t>(rawOpcode) >> 20;
    }
    else if (opcode == 0x23) { // S-type example (store).
        type = InstType::S_TYPE;
        int imm_high = (rawOpcode >> 25) & 0x7F;
        int imm_low  = (rawOpcode >> 7)  & 0x1F;
        immediate = (imm_high << 5) | imm_low;
        // Further decode rs1, rs2, funct3 as needed.
    }
    else if (opcode == 0x63) { // B-type example (branch).
        type = InstType::B_TYPE;
        int imm_12   = (rawOpcode >> 31) & 0x1;
        int imm_11   = (rawOpcode >> 7)  & 0x1;
        int imm_10_5 = (rawOpcode >> 25) & 0x3F;
        int imm_4_1  = (rawOpcode >> 8)  & 0xF;
        immediate = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
    }
    else if (opcode == 0x37 || opcode == 0x17) { // U-type example (LUI, AUIPC).
        type = InstType::U_TYPE;
        info.u.rd  = (rawOpcode >> 7) & 0x1F;
        info.u.imm = rawOpcode & 0xFFFFF000;
    }
    else if (opcode == 0x6F) { // J-type example (JAL).
        type = InstType::J_TYPE;
        info.j.rd = (rawOpcode >> 7) & 0x1F;
        int imm_20    = (rawOpcode >> 31) & 0x1;
        int imm_10_1  = (rawOpcode >> 21) & 0x3FF;
        int imm_11    = (rawOpcode >> 20) & 0x1;
        int imm_19_12 = (rawOpcode >> 12) & 0xFF;
        info.j.imm = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1);
    }
    else {
        type = InstType::UNKNOWN;
        // Optionally print an error or handle unsupported opcodes.
    }
}
