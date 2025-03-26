#include "../header/Instruction.hpp"
#include <cstdlib>
#include <iostream>
#include <bitset>

Instruction::Instruction(const std::string &hex)
    : rawHex(hex), rawOpcode(0), opcode(0), type(InstType::UNKNOWN), id(-1) {
    // Convert the hex string to a 32-bit unsigned integer.
    rawOpcode = std::stoul(hex, nullptr, 16);
    decode();
}
void Instruction::print_opcode(int rawOpcode)
{
    std::bitset<32> bits(rawOpcode);
    std::cout << "Raw opcode (32-bit): " << bits << std::endl;
    
}

void Instruction::decode() {
    opcode = rawOpcode & 0x7F; // bits [6:0]

    if (opcode == 0x33) {
        // --------------------------
        // R-Type (no immediate)
        // --------------------------
        type = InstType::R_TYPE;
        info.r.rd     = (rawOpcode >> 7)  & 0x1F;
        info.r.funct3 = (rawOpcode >> 12) & 0x7;
        info.r.rs1    = (rawOpcode >> 15) & 0x1F;
        info.r.rs2    = (rawOpcode >> 20) & 0x1F;
        info.r.funct7 = (rawOpcode >> 25) & 0x7F;
    }
    else if (opcode == 0x13 || opcode == 0x03 || opcode == 0x67) {
        // --------------------------
        // I-Type (ADDI/LOAD/JALR, etc.)
        // 12-bit signed immediate (bits [31:20])
        // --------------------------
        type = InstType::I_TYPE;
        info.i.rd     = (rawOpcode >> 7) & 0x1F;
        info.i.funct3 = (rawOpcode >> 12) & 0x7;
        info.i.rs1    = (rawOpcode >> 15) & 0x1F;
    
        // Extract the 12-bit immediate, then sign-extend
        int32_t imm_i = (rawOpcode >> 20) & 0xFFF; // bits [31:20]
        if (imm_i & 0x800) { // sign extension check
            imm_i |= 0xFFFFF000;
        }
        info.i.imm = imm_i;
    }
    else if (opcode == 0x23) {
        // --------------------------
        // S-Type (Store)
        // 12-bit signed immediate
        // from bits [31:25] and [11:7]
        // --------------------------
        type = InstType::S_TYPE;
        info.s.rs1    = (rawOpcode >> 15) & 0x1F;
        info.s.rs2    = (rawOpcode >> 20) & 0x1F;
        info.s.funct3 = (rawOpcode >> 12) & 0x7;

        int imm_high  = (rawOpcode >> 25) & 0x7F; // bits [31:25]
        int imm_low   = (rawOpcode >> 7)  & 0x1F; // bits [11:7]
        int32_t imm_s = (imm_high << 5) | imm_low; // 12 bits total
        // Sign-extend if bit 11 is set
        if (imm_s & 0x800) { // 0x800 = 1 << 11
            imm_s |= 0xFFFFF000;
        }
        info.s.imm = imm_s;
    }
    else if (opcode == 0x63) {
        // --------------------------
        // B-Type (Branch)
        // 13-bit signed immediate
        // from bits [31], [11:7], [30:25], [11:8]
        // --------------------------
        type = InstType::B_TYPE;
        info.b.rs1    = (rawOpcode >> 15) & 0x1F;
        info.b.rs2    = (rawOpcode >> 20) & 0x1F;
        info.b.funct3 = (rawOpcode >> 12) & 0x7;

        int imm_12   = (rawOpcode >> 31) & 0x1;  // bit [31]
        int imm_11   = (rawOpcode >> 7)  & 0x1;  // bit [7]
        int imm_10_5 = (rawOpcode >> 25) & 0x3F; // bits [30:25]
        int imm_4_1  = (rawOpcode >> 8)  & 0xF;  // bits [11:8]

        int32_t imm_b = (imm_12 << 12)
                      | (imm_11 << 11)
                      | (imm_10_5 << 5)
                      | (imm_4_1 << 1);
        // Now sign-extend if bit 12 (the top bit) is set
        if (imm_b & 0x1000) { // 0x1000 = 1 << 12
            imm_b |= 0xFFFFE000;
        }
        info.b.imm = imm_b;
    }
    else if (opcode == 0x37 || opcode == 0x17) {
        // --------------------------
        // U-Type (LUI/AUIPC)
        // 20-bit immediate (no sign extension in usual usage)
        // bits [31:12]
        // --------------------------
        type = InstType::U_TYPE;
        info.u.rd  = (rawOpcode >> 7) & 0x1F;
        // Typically, no sign extension for U-type
        info.u.imm = (rawOpcode & 0xFFFFF000);
    }
    else if (opcode == 0x6F) {
        // --------------------------
        // J-Type (JAL)
        // 21-bit signed immediate
        // from bits [31], [19:12], [20], [30:21]
        // --------------------------
        type = InstType::J_TYPE;
        info.j.rd = (rawOpcode >> 7) & 0x1F;

        int imm_20    = (rawOpcode >> 31) & 0x1;   // bit [31]
        int imm_19_12 = (rawOpcode >> 12) & 0xFF;  // bits [19:12]
        int imm_11    = (rawOpcode >> 20) & 0x1;   // bit [20]
        int imm_10_1  = (rawOpcode >> 21) & 0x3FF; // bits [30:21]

        int32_t imm_j = (imm_20    << 20)
                      | (imm_19_12 << 12)
                      | (imm_11    << 11)
                      | (imm_10_1  << 1);
        // Check bit 20 for sign extension
        if (imm_j & 0x100000) { // 0x100000 = 1 << 20
            imm_j |= 0xFFE00000;
        }
        info.j.imm = imm_j;
    }
    else {
        // --------------------------
        // Unsupported / unknown
        // --------------------------
        type = InstType::UNKNOWN;
        std::cout << "Unsupported opcode: " << opcode << std::endl;
        std::cout << "Raw opcode: " << rawOpcode << std::endl;
    }
}


void Instruction::printc_instruction() const {
    if(rawHex == "00000000" || rawHex.empty())
        std::cout << "NOP";
    else
        std::cout << rawHex;
}


void Instruction::print_inst_members()
{
    switch(type)
    {
        case InstType::R_TYPE:
            std::cout << "R TYPE INSTRUCTION" << std::endl;
            std::cout << "rd: " << (int)info.r.rd << std::endl;
            std::cout << "rs1: " << (int)info.r.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.r.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.r.funct3 << std::endl;
            std::cout << "funct7: " << (int)info.r.funct7 << std::endl;
            break;
        case InstType::I_TYPE:
            std::cout << "I TYPE INSTRUCTION" << std::endl;
            std::cout << "rd: " << (int)info.i.rd << std::endl;
            std::cout << "rs1: " << (int)info.i.rs1 << std::endl;
            std::cout << "funct3: " << (int)info.i.funct3 << std::endl;
            std::cout << "imm: " << info.i.imm << std::endl;
            break;
        case InstType::S_TYPE:
            std::cout << "S TYPE INSTRUCTION" << std::endl;
            std::cout << "rs1: " << (int)info.s.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.s.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.s.funct3 << std::endl;
            std::cout << "imm: " << info.s.imm << std::endl;
            break;
        case InstType::B_TYPE:
            std::cout << "B TYPE INSTRUCTION" << std::endl;
            std::cout << "rs1: " << (int)info.b.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.b.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.b.funct3 << std::endl;
            std::cout << "imm: " << info.b.imm << std::endl;
            break;
        case InstType::U_TYPE:
            std::cout << "U TYPE INSTRUCTION" << std::endl;
            std::cout << "rd: " << (int)info.u.rd << std::endl;
            std::cout << "imm: " << info.u.imm << std::endl;
            break;
        case InstType::J_TYPE:
            std::cout << "J TYPE INSTRUCTION" << std::endl;
            std::cout << "rd: " << (int)info.j.rd << std::endl;
            std::cout << "imm: " << info.j.imm << std::endl;
            break;
        default:
            std::cout << "Unknown instruction type" << std::endl;
            break;
    }
}

