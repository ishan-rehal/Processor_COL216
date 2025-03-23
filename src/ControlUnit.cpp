#include "ControlUnit.hpp"

ControlSignals ControlUnit::decode(const Instruction& inst) {
    ControlSignals signals = {false, false, false, false, 0};
    // For example, if the instruction is I-type (e.g., ADDI)
    if (inst.type == InstType::I_TYPE) {
        signals.regWrite = true;
        signals.memRead  = false;
        signals.memWrite = false;
        signals.branch   = false;
        signals.aluOp    = 0; // Assume 0 represents an ADD operation.
    }
    // Extend with decoding for other instruction types (R, S, B, etc.)
    return signals;
}
