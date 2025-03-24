#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <cstdint>

// Supported instruction types.
enum class InstType { R_TYPE, I_TYPE, S_TYPE, B_TYPE, U_TYPE, J_TYPE, NOP, UNKNOWN };

class Instruction {
public:
    // Raw instruction data.
    std::string rawHex;   // The 8-character hexadecimal string.
    uint32_t rawOpcode;   // The converted 32-bit opcode.
    
    // Decoded fields.
    uint8_t opcode;       // 7-bit opcode field.

    
    InstType type;        // Type of instruction.

    // For modularity, you can use a union or structs for different formats.
    union {
        struct { // I-type format (e.g., ADDI)
            uint8_t rd;
            uint8_t rs1;
            uint8_t funct3;
            int imm;
        } i;
        struct { // R-type format (e.g., ADD, SUB)
            uint8_t rd;
            uint8_t rs1;
            uint8_t rs2;
            uint8_t funct3;
            uint8_t funct7;
        } r;

        struct { // S-type format (e.g., STORE instructions)
            uint8_t rs1;
            uint8_t rs2;
            uint8_t funct3;
            int imm;
        } s;

        struct { // B-type format (e.g., Branch instructions)
            uint8_t rs1;
            uint8_t rs2;
            uint8_t funct3;
            int imm;
        } b;

        struct { // U-type format (e.g., LUI)
            uint8_t rd;
            int imm;
        } u;
        
        struct { // J-type format (e.g., JAL)
            uint8_t rd;
            int imm;
        } j;
        // You can add S-type, B-type, etc.
    } info;

    // Constructors.
    Instruction() : rawHex("00000000"), rawOpcode(0), opcode(0), type(InstType::NOP) {}
                    
    Instruction(const std::string &hex);

    // Decodes the raw opcode into fields.
    void decode();
    void print_opcode(int rawOpcode);
    void printc_instruction() const;
    void instruction_copy(Instruction &inst);
    void print_inst_members();
};

#endif // INSTRUCTION_HPP
