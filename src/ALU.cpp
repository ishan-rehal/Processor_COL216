#include "ALU.hpp"

int ALU::add(int op1, int op2) {
    return op1 + op2;
}

int ALU::sub(int op1, int op2) {
    return (op1 - op2);
}

int ALU::mul(int op1, int op2) {
    return op1 * op2;
}

int ALU::div(int op1, int op2) {
    // For division, check for divide-by-zero (could also set an error flag)
    if (op2 == 0) {
        // In a real processor, this might trigger an exception.
        return 0; 
    }
    return op1 / op2;
}
