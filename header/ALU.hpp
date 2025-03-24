#ifndef ALU_HPP
#define ALU_HPP

#include <cstdint>

class ALU {
public:
    // Basic arithmetic operations
    static int add(int op1, int op2);
    static int sub(int op1, int op2);
    static int mul(int op1, int op2);
    static int div(int op1, int op2); // simple unsigned division; check for zero
    // You can also add logical operations, shifts, and comparison functions
};

#endif // ALU_HPP
