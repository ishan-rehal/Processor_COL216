// test_instruction.cpp
#include <cassert>
#include <iostream>
#include "../header/Instruction.hpp"  // Assumes Instruction.hpp defines Instruction, InstType, and the union 'info'

int main() {
    // -----------------------
    // Test R-type Instruction
    // Construct an R-type instruction with:
    //  - opcode = 0x33 (R-type)
    //  - rd = 5, funct3 = 0, rs1 = 1, rs2 = 2, funct7 = 0
    // Encoding: rawOpcode = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode
    //           = (0 << 25) | (2 << 20) | (1 << 15) | (0 << 12) | (5 << 7) | 0x33 = 0x002082B3
    {
        Instruction inst("002082B3");
        assert(inst.type == InstType::R_TYPE);
        assert(inst.info.r.rd     == 5);
        assert(inst.info.r.funct3 == 0);
        assert(inst.info.r.rs1    == 1);
        assert(inst.info.r.rs2    == 2);
        assert(inst.info.r.funct7 == 0);
    }

    // -----------------------
    // Test I-type Instruction (e.g. ADDI)
    // Construct an I-type instruction with:
    //  - opcode = 0x13 (ADDI)
    //  - rd = 5, funct3 = 0, rs1 = 1, immediate = 10
    // Encoding: immediate in bits[31:20] is 10, so:
    //           rawOpcode = (10 << 20) | (1 << 15) | (0 << 12) | (5 << 7) | 0x13 = 0x00A08293
    {
        Instruction inst("00A08293");
        assert(inst.type == InstType::I_TYPE);
        assert(inst.info.i.rd     == 5);
        assert(inst.info.i.funct3 == 0);
        assert(inst.info.i.rs1    == 1);
        // The immediate is sign-extended via: static_cast<int32_t>(rawOpcode) >> 20
        assert(inst.info.i.imm    == 10);
    }

    // -----------------------
    // Test S-type Instruction (e.g. Store)
    // Construct an S-type instruction with:
    //  - opcode = 0x23 (Store)
    //  - rs1 = 3, rs2 = 2, funct3 = 0, immediate = 20
    // Immediate is split: imm_high = immediate >> 5 = 0 and imm_low = immediate & 0x1F = 20.
    // Encoding: rawOpcode = (imm_high << 25) | (rs2 << 20) | (rs1 << 15)
    //           | (funct3 << 12) | (imm_low << 7) | opcode = 0x00218A23
    {
        Instruction inst("00218A23");
        assert(inst.type == InstType::S_TYPE);
        assert(inst.info.s.rs1    == 3);
        assert(inst.info.s.rs2    == 2);
        assert(inst.info.s.funct3 == 0);
        assert(inst.info.s.imm    == 20);
    }

    // -----------------------
    // Test B-type Instruction (e.g. Branch)
    // Construct a B-type instruction with:
    //  - opcode = 0x63 (Branch)
    //  - rs1 = 1, rs2 = 2, funct3 = 0
    //  - immediate = 16, encoded as:
    //       imm_12 = 0, imm_11 = 0, imm_10_5 = 0, imm_4_1 = 8  (since 8 << 1 = 16)
    // Encoding: rawOpcode = (imm_12 << 31) | (imm_10_5 << 25) | (rs2 << 20) | (rs1 << 15)
    //           | (funct3 << 12) | (imm_4_1 << 8) | (imm_11 << 7) | opcode = 0x00208863
    {
        Instruction inst("00208863");
        assert(inst.type == InstType::B_TYPE);
        assert(inst.info.b.rs1    == 1);
        assert(inst.info.b.rs2    == 2);
        assert(inst.info.b.funct3 == 0);
        assert(inst.info.b.imm    == 16);
    }

    // -----------------------
    // Test U-type Instruction (e.g. LUI)
    // Construct a U-type instruction with:
    //  - opcode = 0x37 (LUI)
    //  - rd = 5, and imm = 0x12345000 (the lower 12 bits are zero by definition)
    // Encoding: rawOpcode = (0x12345 << 12) | (5 << 7) | 0x37 = 0x123452B7
    {
        Instruction inst("123452B7");
        assert(inst.type == InstType::U_TYPE);
        assert(inst.info.u.rd  == 5);
        assert(inst.info.u.imm == 0x12345000);
    }

    // -----------------------
    // Test J-type Instruction (e.g. JAL)
    // Construct a J-type instruction with:
    //  - opcode = 0x6F (JAL)
    //  - rd = 1
    //  - immediate = 2, which is encoded (note the immediate encoding is noncontiguous):
    //       Let imm_20 = 0, imm_19_12 = 0, imm_11 = 0, and imm_10_1 = 1 (since 1 << 1 = 2).
    // Encoding: rawOpcode = (imm_20 << 31) | (imm_10_1 << 21) | (imm_11 << 20)
    //           | (imm_19_12 << 12) | (rd << 7) | opcode = 0x002000EF
    {
        Instruction inst("002000EF");
        assert(inst.type == InstType::J_TYPE);
        assert(inst.info.j.rd  == 1);
        assert(inst.info.j.imm == 2);
    }

    // -----------------------
    // Test Unknown Instruction
    // Use an opcode that does not match any known type.
    {
        Instruction inst("FFFFFFFF");
        assert(inst.type == InstType::UNKNOWN);
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
// End of test_instruction.cpp