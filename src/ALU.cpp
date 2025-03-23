#include "ALU.hpp"

uint32_t ALU::add(uint32_t op1, uint32_t op2) {
    return op1 + op2;
}

uint32_t ALU::sub(uint32_t op1, uint32_t op2) {
    return op1 - op2;
}

uint32_t ALU::mul(uint32_t op1, uint32_t op2) {
    return op1 * op2;
}

uint32_t ALU::div(uint32_t op1, uint32_t op2) {
    // For division, check for divide-by-zero (could also set an error flag)
    if (op2 == 0) {
        // In a real processor, this might trigger an exception.
        return 0; 
    }
    return op1 / op2;
}
