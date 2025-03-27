#include "../header/ALU.hpp"

int ALU::add(int op1, int op2) {
    return op1 + op2;
}

int ALU::sub(int op1, int op2) {
    return op1 - op2;
}

int ALU::mul(int op1, int op2) {
    return op1 * op2;
}

int ALU::div(int op1, int op2) {
    if (op2 == 0) {
        return 0; // Handle divide-by-zero appropriately.
    }
    return op1 / op2;
}

int ALU::sll(int op1, int op2) {
    // Use lower 5 bits of op2 as shift amount.
    return op1 << (op2 & 0x1F);
}

int ALU::srl(int op1, int op2) {
    return static_cast<unsigned int>(op1) >> (op2 & 0x1F);
}

int ALU::sra(int op1, int op2) {
    return op1 >> (op2 & 0x1F);
}
