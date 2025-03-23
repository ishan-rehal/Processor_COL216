#ifndef ALU_HPP
#define ALU_HPP

#include <cstdint>

class ALU {
public:
    // Basic arithmetic operations
    static uint32_t add(uint32_t op1, uint32_t op2);
    static uint32_t sub(uint32_t op1, uint32_t op2);
    static uint32_t mul(uint32_t op1, uint32_t op2);
    static uint32_t div(uint32_t op1, uint32_t op2); // simple unsigned division; check for zero
    // You can also add logical operations, shifts, and comparison functions
};

#endif // ALU_HPP
