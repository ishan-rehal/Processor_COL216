#ifndef ALU_HPP
#define ALU_HPP

#include <cstdint>

class ALU {
public:
    // Performs addition.
    static uint32_t add(uint32_t op1, uint32_t op2);
    
    // Performs subtraction.
    static uint32_t subtract(uint32_t op1, uint32_t op2);
    
    // You can add more operations as needed (AND, OR, etc.)
};

#endif // ALU_HPP
