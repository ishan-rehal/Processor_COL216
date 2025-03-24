#include "../header/Instruction.hpp"
#include <cstdlib>
#include <iostream>
#include <bitset>

Instruction::Instruction(const std::string &hex)
    : rawHex(hex), rawOpcode(0), opcode(0), type(InstType::UNKNOWN) {
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
        int immediate = (imm_high << 5) | imm_low;
        // Further decode rs1, rs2, funct3 as needed.
        info.s.rs1 = (rawOpcode >> 15) & 0x1F;
        info.s.rs2 = (rawOpcode >> 20) & 0x1F;
        info.s.funct3 = (rawOpcode >> 12) & 0x7;
        info.s.imm = immediate;

    }
    else if (opcode == 0x63) { // B-type example (branch).
        type = InstType::B_TYPE;
        int imm_12   = (rawOpcode >> 31) & 0x1;
        int imm_11   = (rawOpcode >> 7)  & 0x1;
        int imm_10_5 = (rawOpcode >> 25) & 0x3F;
        int imm_4_1  = (rawOpcode >> 8)  & 0xF;
        int immediate = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1);
        info.b.rs1 = (rawOpcode >> 15) & 0x1F;
        info.b.rs2 = (rawOpcode >> 20) & 0x1F;
        info.b.funct3 = (rawOpcode >> 12) & 0x7;
        info.b.imm = immediate;

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
            std::cout << "rd: " << (int)info.r.rd << std::endl;
            std::cout << "rs1: " << (int)info.r.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.r.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.r.funct3 << std::endl;
            std::cout << "funct7: " << (int)info.r.funct7 << std::endl;
            break;
        case InstType::I_TYPE:
            std::cout << "rd: " << (int)info.i.rd << std::endl;
            std::cout << "rs1: " << (int)info.i.rs1 << std::endl;
            std::cout << "funct3: " << (int)info.i.funct3 << std::endl;
            std::cout << "imm: " << info.i.imm << std::endl;
            break;
        case InstType::S_TYPE:
            std::cout << "rs1: " << (int)info.s.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.s.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.s.funct3 << std::endl;
            std::cout << "imm: " << info.s.imm << std::endl;
            break;
        case InstType::B_TYPE:
            std::cout << "rs1: " << (int)info.b.rs1 << std::endl;
            std::cout << "rs2: " << (int)info.b.rs2 << std::endl;
            std::cout << "funct3: " << (int)info.b.funct3 << std::endl;
            std::cout << "imm: " << info.b.imm << std::endl;
            break;
        case InstType::U_TYPE:
            std::cout << "rd: " << (int)info.u.rd << std::endl;
            std::cout << "imm: " << info.u.imm << std::endl;
            break;
        case InstType::J_TYPE:
            std::cout << "rd: " << (int)info.j.rd << std::endl;
            std::cout << "imm: " << info.j.imm << std::endl;
            break;
        default:
            std::cout << "Unknown instruction type" << std::endl;
            break;
    }
}

