#ifndef ALU_HPP
#define ALU_HPP


class ALU {
public:
    static int add(int op1, int op2);
    static int sub(int op1, int op2);
    static int mul(int op1, int op2);
    static int div(int op1, int op2);
    static int sll(int op1, int op2);
    static int srl(int op1, int op2);
    static int sra(int op1, int op2);
};

#endif
