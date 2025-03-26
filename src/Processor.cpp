// Processor.cpp
#include "../header/Processor.hpp"
#include "../header/ControlUnit.hpp"
#include "../header/ALU.hpp"
#include <iostream>
#include <iomanip>
#include <string>

// Constructor: initialize registers, PC, pipeline latches, and the stack memory.
Processor::Processor(const std::vector<std::string>& instructionsHex, bool forwarding, int totalCycleCount)
    : PC(0), forwardingEnabled(forwarding),stallIF(false),stallNeeded(false), totalCycleCount(totalCycleCount),
    currentCycle(0), headerPrinted(false)  
{
    // Load instructions from hex strings.
    for (const auto &hex : instructionsHex) {
        Instruction inst(hex);
        instructionMemory.push_back(inst);
    }

    // Assign a unique id to each instruction.
    for (size_t i = 0; i < instructionMemory.size(); ++i) {
        instructionMemory[i].id = i;
    }

    regs.resize(32, 0);  // Initialize 32 registers to 0.

    // Initialize pipeline registers to NOP.
    Instruction nop;
    nop.type = InstType::NOP;
    if_id.instruction = nop;
    id_ex.instruction = nop;
    ex_mem.instruction = nop;
    mem_wb.instruction = nop;
    
    next_if_id = if_id;
    next_id_ex = id_ex;
    next_ex_mem = ex_mem;
    next_mem_wb = mem_wb;
    
    // NEW: Initialize stack memory to 1024 bytes (all zeros)
    stack_memory.resize(1024, 0);

     // Initialize the pipeline log: one row per instruction, with one cell per cycle.
     pipelineLog.resize(instructionMemory.size(), std::vector<std::string>(totalCycleCount, ""));
}

// Logging helper: record the given stage name for the instruction at the current cycle.
void Processor::logInstructionStage(const Instruction &instr, const std::string &stage) {
    if (instr.type == InstType::NOP || instr.id < 0 || currentCycle >= totalCycleCount)
        return;
    std::string &entry = pipelineLog[instr.id][currentCycle];

    // If the incoming stage is "-", only update if nothing meaningful is logged.
    if (stage == "-") {
        if (!entry.empty() && entry != "-") {
            // A valid stage (e.g., "ID") is already logged; ignore the dash.
            return;
        } else {
            // If nothing or only a dash is present, set it as dash.
            entry = "-";
            return;
        }
    }

    // For non-dash stages, if the current entry is empty or just a dash, replace it.
    if (entry.empty() || entry == "-") {
        entry = stage;
    } else {
        // Otherwise, if this stage isn't already recorded in the entry, append it.
        if (entry.find(stage) == std::string::npos) {
            entry += "/" + stage;
        }
    }
}



// Print the header row with cycle numbers.
void Processor::printPipelineLogHeader() const {
    std::cout << std::setw(6) << " " << ":";
    for (int i = 0; i < totalCycleCount; ++i) {
        std::cout << std::setw(10) << ("C" + std::to_string(i+1));
    }
    std::cout << std::endl;
}

// Print one instruction’s log row (e.g., "I1 :  IF   ID   EX   MEM   WB ...")
void Processor::printInstructionLog(int instrId) const {
    if (instrId < 0 || instrId >= (int)pipelineLog.size()) return;
    std::cout << "I" << (instrId + 1) << std::setw(5) << ":";
    for (const auto &cell : pipelineLog[instrId]) {
        std::cout << std::setw(10) << cell;
    }
    std::cout << std::endl;
}




// Helper to get the destination register for any instruction type.
// Returns 0 if the instruction does not have a destination register.
uint8_t Processor::getRD(const Instruction &inst) {
    switch (inst.type) {
        case InstType::R_TYPE:
            return inst.info.r.rd;
        case InstType::I_TYPE:
            return inst.info.i.rd;
        case InstType::U_TYPE:
            return inst.info.u.rd;
        case InstType::J_TYPE:
            return inst.info.j.rd;
        default:
            // S_TYPE, B_TYPE, NOP, UNKNOWN have no rd
            return 0;
    }
}





// -------------------------
// Fetch Stage (with cycle parameter)
// -------------------------
void Processor::fetch(int cycle) {
    if (cycle == 0) {
        if (PC / 4 < instructionMemory.size()) {
            // Normal fetch
            next_if_id.instruction = instructionMemory[PC / 4];
            next_if_id.pc = PC;
            std::cout << "Fetching instruction: " << PC / 4 << std::endl;
            logInstructionStage(next_if_id.instruction, "IF");
        }
        else {
            // Past the end of instructions => keep fetching NOP
            Instruction nop;
            nop.type = InstType::NOP;
            next_if_id.instruction = nop;
            next_if_id.pc = PC;
        }
    }
}

void Processor::decode(int cycle) {
    // Decode logic runs in the second half of the pipeline cycle
    if (cycle == 1) {
        
        // if_id.instruction.printc_instruction();
        // std::cout << std::endl;

        // 1) Decode raw fields if not NOP
        if (if_id.instruction.type != InstType::NOP) {
            if_id.instruction.decode();
            // if(if_id.instruction.type == InstType::R_TYPE)
            // {
            //     std::cout << "Decoded the R-type instruction: ";
            //     if_id.instruction.print_inst_members();
            // }
        }
        std :: cout << "Cycle No: " << currentCycle << std::endl;
        std::cout << "Decoding instruction: ";
        if_id.instruction.print_inst_members();
        std::cout << std::endl;

        // If not a NOP and no stall, log "ID".
        if (if_id.instruction.type != InstType::NOP && !stallNeeded) {
            logInstructionStage(if_id.instruction, "ID");
        }


        // Identify which registers the current IF/ID instruction needs
        bool usesRS1 = false, usesRS2 = false;
        uint8_t neededRS1 = 0, neededRS2 = 0;

        switch (if_id.instruction.type) {
            case InstType::R_TYPE:
                neededRS1 = if_id.instruction.info.r.rs1;
                neededRS2 = if_id.instruction.info.r.rs2;
                usesRS1 = true;
                usesRS2 = true;
                break;
            case InstType::I_TYPE:
                neededRS1 = if_id.instruction.info.i.rs1;
                usesRS1 = true;
                break;
            case InstType::S_TYPE:
                neededRS1 = if_id.instruction.info.s.rs1;
                neededRS2 = if_id.instruction.info.s.rs2;
                usesRS1 = true;
                usesRS2 = true;
                break;
            case InstType::B_TYPE:
                neededRS1 = if_id.instruction.info.b.rs1;
                neededRS2 = if_id.instruction.info.b.rs2;
                usesRS1 = true;
                usesRS2 = true;
                break;
            default:
                // U_TYPE, J_TYPE, NOP, UNKNOWN => no registers to read
                break;
        }

        if (!forwardingEnabled) {
            // Hazard detection when forwarding is disabled.
            stallNeeded = false;
        
            // (a) Check ID/EX stage for potential hazards.
            if (id_ex.regWrite) {
                uint8_t rd_idex = getRD(id_ex.instruction);
                if (rd_idex != 0) {
                    if ((usesRS1 && rd_idex == neededRS1) ||
                        (usesRS2 && rd_idex == neededRS2)) {
                        stallNeeded = true;
                    }
                }
            }
        
            // (b) Check EX/MEM stage for potential hazards.
            if (ex_mem.regWrite) {
                uint8_t rd_exmem = getRD(ex_mem.instruction);
                if (rd_exmem != 0) {
                    if ((usesRS1 && rd_exmem == neededRS1) ||
                        (usesRS2 && rd_exmem == neededRS2)) {
                        stallNeeded = true;
                    }
                }
            }
        
            // If a hazard is detected, insert a NOP in the ID/EX latch and stall IF.
            if (stallNeeded) {
                logInstructionStage(if_id.instruction, "-");
                Instruction nop;
                nop.type = InstType::NOP;
                next_id_ex.instruction = nop;
                next_id_ex.regWrite    = false;
                next_id_ex.memRead     = false;
                next_id_ex.memWrite    = false;
                next_id_ex.branch      = false;
                next_id_ex.aluOp       = ALUOp::NONE;
                next_id_ex.rs1Val      = 0;
                next_id_ex.rs2Val      = 0;
                next_id_ex.imm         = 0;
        
                stallIF = true;
                return;
            } else {
                // No hazard: simply log a "-" (or you could log "ID" if preferred).
                logInstructionStage(if_id.instruction, "-");
            }
        } else {
            // Forwarding is enabled.
            // When forwarding is enabled, only check for load–use hazards.
            stallNeeded = false;
            if (id_ex.memRead) {
                uint8_t rd_idex = getRD(id_ex.instruction);
                if (rd_idex != 0) {
                    if ((usesRS1 && rd_idex == neededRS1) ||
                        (usesRS2 && rd_idex == neededRS2)) {
                        stallNeeded = true;
                    }
                }
            }
            if (stallNeeded) {
                logInstructionStage(if_id.instruction, "-");
                Instruction nop;
                nop.type = InstType::NOP;
                next_id_ex.instruction = nop;
                next_id_ex.regWrite    = false;
                next_id_ex.memRead     = false;
                next_id_ex.memWrite    = false;
                next_id_ex.branch      = false;
                next_id_ex.aluOp       = ALUOp::NONE;
                next_id_ex.rs1Val      = 0;
                next_id_ex.rs2Val      = 0;
                next_id_ex.imm         = 0;
                stallIF = true;
                return;
            } else {
                logInstructionStage(if_id.instruction, "-");
            }
        }
        
        
        // 4) No stall => normal decode logic from the ControlUnit
        ControlSignals signals = ControlUnit::decode(if_id.instruction);

        // Copy instruction + PC into ID/EX
        next_id_ex.pc          = if_id.pc;
        next_id_ex.instruction = if_id.instruction;
        next_id_ex.regWrite    = signals.regWrite;
        next_id_ex.memRead     = signals.memRead;
        next_id_ex.memWrite    = signals.memWrite;
        next_id_ex.branch      = signals.branch;
        next_id_ex.aluOp       = signals.aluOp;

        // -------------------------------------------------------
        // Register read logic + special handling for branch/jump
        // -------------------------------------------------------
        switch (if_id.instruction.type) {
            // -----------------
            // R-TYPE
            // -----------------
            case InstType::R_TYPE:
                next_id_ex.rs1Val = regs[if_id.instruction.info.r.rs1];
                next_id_ex.rs2Val = regs[if_id.instruction.info.r.rs2];
                next_id_ex.imm    = 0;
                break;

            // -----------------
            // I-TYPE
            // -----------------
            case InstType::I_TYPE:
                next_id_ex.rs1Val = regs[if_id.instruction.info.i.rs1];
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = if_id.instruction.info.i.imm;
                if (if_id.instruction.opcode == 0x67) {  // JALR
                    int jumpTarget = next_id_ex.rs1Val + next_id_ex.imm;
                    jumpTarget &= ~1; // Optionally clear the least significant bit (address alignment)
                    regs[if_id.instruction.info.i.rd] = if_id.pc + 4;  // Save the link address (return address) in rd
                    PC = jumpTarget - 4;  // Update PC to the jump target and -4 becasue PC + 4 will take place anyways
                    Instruction nop;
                    nop.type = InstType::NOP;
                    next_id_ex.instruction = nop;  // Flush ID/EX
                    next_if_id.instruction = nop;  // Flush IF/ID
                    return;  // Stop further decode processing for this cycle
                }
                break;

            // -----------------
            // S-TYPE
            // -----------------
            case InstType::S_TYPE:
                next_id_ex.rs1Val = regs[if_id.instruction.info.s.rs1];
                next_id_ex.rs2Val = regs[if_id.instruction.info.s.rs2];
                next_id_ex.imm    = if_id.instruction.info.s.imm;
                break;

            // -----------------
            // B-TYPE (Branches)
            // -----------------
            case InstType::B_TYPE: {
                uint32_t rs1Val = regs[if_id.instruction.info.b.rs1];
                uint32_t rs2Val = regs[if_id.instruction.info.b.rs2];
                int32_t offset  = if_id.instruction.info.b.imm;

                next_id_ex.rs1Val = rs1Val;
                next_id_ex.rs2Val = rs2Val;
                next_id_ex.imm    = offset;

                uint8_t f3 = if_id.instruction.info.b.funct3;
                bool branchTaken = false;

                switch (f3) {
                    case 0: // BEQ
                        branchTaken = (rs1Val == rs2Val);
                        break;
                    case 1: // BNE
                        branchTaken = (rs1Val != rs2Val);
                        break;
                    case 4: // BLT
                        branchTaken = ((int32_t)rs1Val < (int32_t)rs2Val);
                        break;
                    case 5: // BGE
                        branchTaken = ((int32_t)rs1Val >= (int32_t)rs2Val);
                        break;
                    case 6: // BLTU
                        branchTaken = (rs1Val < rs2Val);
                        break;
                    case 7: // BGEU
                        branchTaken = (rs1Val >= rs2Val);
                        break;
                    default:
                        // If unknown branch, treat as not taken
                        break;
                }

                // Update PC in the decode stage
                if (branchTaken) {
                    PC = if_id.pc + offset;
                } else {
                    PC = if_id.pc + 4;
                }

                // Flush the pipeline: send NOP to ID/EX
                Instruction nop;
                // nop.type = InstType::NOP;
                // next_id_ex.instruction = nop;
                // next_id_ex.regWrite    = false;
                // next_id_ex.memRead     = false;
                // next_id_ex.memWrite    = false;
                // next_id_ex.branch      = false;
                // next_id_ex.aluOp       = ALUOp::NONE;
                // next_id_ex.rs1Val      = 0;
                // next_id_ex.rs2Val      = 0;
                // next_id_ex.imm         = 0;
                next_id_ex.instruction = if_id.instruction;
                next_id_ex.regWrite    = false;
                next_id_ex.memRead     = false;
                next_id_ex.memWrite    = false;
                next_id_ex.branch      = false;
                next_id_ex.aluOp       = ALUOp::NONE;
                next_id_ex.rs1Val      = 0;
                next_id_ex.rs2Val      = 0;
                next_id_ex.imm         = 0;


                // ALSO flush IF/ID so we won't re-decode the same branch
                next_if_id.instruction = nop;
                // next_if_id.instruction = if_id.instruction;
                next_if_id.pc          = PC;

                // DO NOT stall next cycle — we want to fetch the new instruction
                stallIF = false;

                return; // Done handling the branch
            }

            // -----------------
            // U-TYPE
            // -----------------
            case InstType::U_TYPE:
                next_id_ex.rs1Val = 0;
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = if_id.instruction.info.u.imm;
                break;

            // -----------------
            // J-TYPE (e.g. JAL)
            // -----------------
            case InstType::J_TYPE: {
                int32_t offset = if_id.instruction.info.j.imm;
                // Unconditional jump
                // -4 because +4 will take place in UpdateLatch
                PC = if_id.pc + offset -4;

                // Flush pipeline: send NOP to ID/EX
                Instruction nop;
                nop.type = InstType::NOP;
                next_id_ex.instruction = nop;
                next_id_ex.regWrite    = false;
                next_id_ex.memRead     = false;
                next_id_ex.memWrite    = false;
                next_id_ex.branch      = false;
                next_id_ex.aluOp       = ALUOp::NONE;
                next_id_ex.rs1Val      = 0;
                next_id_ex.rs2Val      = 0;
                next_id_ex.imm         = 0;

                // ALSO flush IF/ID so we won't re-decode the same jump
                next_if_id.instruction = nop;
                next_if_id.pc          = PC;

                // DO NOT stall next cycle — let new fetch proceed
                stallIF = false;

                return; 
            }

            // -----------------
            // NOP / UNKNOWN
            // -----------------
            default:
                next_id_ex.rs1Val = 0;
                next_id_ex.rs2Val = 0;
                next_id_ex.imm    = 0;
                break;
        }
    }
}



// -------------------------
// Execute Stage (with cycle parameter)
// -------------------------
void Processor::execute(int cycle) {
    if (cycle == 0) {
        // std::cout << "[DEBUG] EX stage: Instruction = ";
        // id_ex.instruction.printc_instruction();
        // std::cout << "\n  ALUOp = " << (int)id_ex.aluOp
        //           << ", rs1Val = " << id_ex.rs1Val
        //           << ", rs2Val = " << id_ex.rs2Val
        //           << ", imm = " << id_ex.imm << std::endl;
        uint32_t operand1 = id_ex.rs1Val;
        uint32_t operand2 = 0;

        // Set operand2 based on instruction type.
        switch (id_ex.instruction.type) {
            case InstType::R_TYPE:
                operand2 = id_ex.rs2Val;
                break;
            case InstType::I_TYPE:
                operand2 = id_ex.imm;
                break;
            case InstType::S_TYPE:
                operand2 = id_ex.imm;
                break;
            case InstType::B_TYPE:
                operand2 = id_ex.rs2Val;
                break;
            case InstType::U_TYPE:
                // For U-type, operand2 may not be used by the ALU.
                operand2 = id_ex.imm;
                break;
            case InstType::J_TYPE: // ye PC update mei krna h
                // For J-type (e.g., JAL), we compute the jump target.
                // The jump target is computed as current PC + immediate.
                // Although we set operand2 here, the main purpose is to compute branchTarget.
                operand2 = id_ex.imm;
                break;
            default:
                operand2 = id_ex.imm;
                break;
        }

         // If forwarding is enabled, override operands if a later stage holds the updated value.
        if (forwardingEnabled) {
            uint8_t rs1 = 0, rs2 = 0;
            bool useRS2 = false;
            switch (id_ex.instruction.type) {
                case InstType::R_TYPE:
                    rs1 = id_ex.instruction.info.r.rs1;
                    rs2 = id_ex.instruction.info.r.rs2;
                    useRS2 = true;
                    break;
                case InstType::I_TYPE:
                    rs1 = id_ex.instruction.info.i.rs1;
                    break;
                case InstType::S_TYPE:
                    rs1 = id_ex.instruction.info.s.rs1;
                    rs2 = id_ex.instruction.info.s.rs2;
                    useRS2 = true;
                    break;
                case InstType::B_TYPE:
                    rs1 = id_ex.instruction.info.b.rs1;
                    rs2 = id_ex.instruction.info.b.rs2;
                    useRS2 = true;
                    break;
                default:
                    break;
            }

            // Forward for operand1 (rs1)
            if (rs1 != 0) {
                if (ex_mem.regWrite && (getRD(ex_mem.instruction) == rs1)) {
                    operand1 = ex_mem.aluResult;
                } else if (mem_wb.regWrite && (getRD(mem_wb.instruction) == rs1)) {
                    operand1 = mem_wb.writeData;
                }
            }

            // Forward for operand2 (rs2), if applicable.
            if (useRS2 && rs2 != 0) {
                if (ex_mem.regWrite && (getRD(ex_mem.instruction) == rs2)) {
                    operand2 = ex_mem.aluResult;
                } else if (mem_wb.regWrite && (getRD(mem_wb.instruction) == rs2)) {
                    operand2 = mem_wb.writeData;
                }
            }
        }





        int aluResult = 0;
        // Perform the ALU operation as needed.
        switch (id_ex.aluOp) {
            case ALUOp::ADD:
                aluResult = ALU::add(operand1, operand2);
                break;
            case ALUOp::SUB:
                // std::cout << "Subtracting " << operand1 << " - " << operand2 << std::endl;
                aluResult = ALU::sub(operand1, operand2);
                break;
            case ALUOp::MUL:
                aluResult = ALU::mul(operand1, operand2);
                break;
            case ALUOp::DIV:
                aluResult = ALU::div(operand1, operand2);
                break;
            default:
                aluResult = 0;
                break;
        }
        
        // For branch or jump instructions (B-type and J-type),
        // compute the branch/jump target address.
        next_ex_mem.branchTarget = id_ex.pc + id_ex.imm;
        
        // Prepare next EX/MEM latch.
        next_ex_mem.aluResult = aluResult;
        next_ex_mem.rs2Val = id_ex.rs2Val;
        next_ex_mem.regWrite = id_ex.regWrite;
        next_ex_mem.memRead = id_ex.memRead;
        next_ex_mem.memWrite = id_ex.memWrite;
        next_ex_mem.branch = id_ex.branch;
        next_ex_mem.instruction = id_ex.instruction;
        logInstructionStage(id_ex.instruction, "EX");

    }
    // Second half: no additional work in execute stage.
}



// -------------------------
// Memory Access Stage (with cycle parameter)
// -------------------------
void Processor::memAccess(int cycle) {
    // Perform the memory operation in the whole cycle.
    uint32_t addr = ex_mem.aluResult;

    // If this is a store operation:
    if (ex_mem.memWrite) {
        uint32_t value = ex_mem.rs2Val;
        // When forwarding is enabled, forward the value from MEM/WB if available.
        if (forwardingEnabled) {
            uint8_t store_rs2 = ex_mem.instruction.info.s.rs2;
            if (store_rs2 != 0 && mem_wb.regWrite && (getRD(mem_wb.instruction) == store_rs2)) {
                value = mem_wb.writeData;
            }
        }
        uint8_t funct3 = ex_mem.instruction.info.s.funct3;
        switch (funct3) {
            case 0: // SB: Store Byte
                if (addr < stack_memory.size())
                    stack_memory[addr] = value & 0xFF;
                break;
            case 1: // SH: Store Halfword
                if (addr + 1 < stack_memory.size()) {
                    stack_memory[addr] = value & 0xFF;
                    stack_memory[addr + 1] = (value >> 8) & 0xFF;
                }
                break;
            case 2: // SW: Store Word
                if (addr + 3 < stack_memory.size()) {
                    stack_memory[addr]     = value & 0xFF;
                    stack_memory[addr + 1] = (value >> 8) & 0xFF;
                    stack_memory[addr + 2] = (value >> 16) & 0xFF;
                    stack_memory[addr + 3] = (value >> 24) & 0xFF;
                }
                break;
            case 3: // SD: Store Doubleword
                if (addr + 7 < stack_memory.size()) {
                    for (int i = 0; i < 8; i++) {
                        stack_memory[addr + i] = (value >> (8 * i)) & 0xFF;
                    }
                }
                break;
            default:
                // Unsupported store type.
                break;
        }
        next_mem_wb.writeData   = 0;
        next_mem_wb.regWrite    = false;
        next_mem_wb.instruction = ex_mem.instruction;
    }
    // Else if this is a load operation:
    else if (ex_mem.memRead) {
        uint32_t data = 0;
        uint8_t funct3 = ex_mem.instruction.info.i.funct3;
        switch (funct3) {
            case 0: { // LB: Load Byte (sign-extended)
                // std::cout << "Loading Byte from : " << addr << std::endl ;
                if (addr < stack_memory.size()) {
                    int8_t byte = static_cast<int8_t>(stack_memory[addr]);
                    data = static_cast<int32_t>(byte);
                }
                break;
            }
            case 4: { // LBU: Load Byte Unsigned
                // std::cout << "Loading Byte Unsigned from : " << addr << std::endl ;
                if (addr < stack_memory.size()) {
                    data = stack_memory[addr];
                }
                break;
            }
            case 1: { // LH: Load Halfword (sign-extended)
                // std::cout << "Loading HW from : " << addr << std::endl ;
                if (addr + 1 < stack_memory.size()) {
                    int16_t half = static_cast<int16_t>(
                        stack_memory[addr] | (stack_memory[addr + 1] << 8)
                    );
                    data = static_cast<int32_t>(half);
                }
                break;
            }
            case 5: { // LHU: Load Halfword Unsigned
                // std::cout << "Loading HWU from : " << addr << std::endl ;
                if (addr + 1 < stack_memory.size()) {
                    data = stack_memory[addr] | (stack_memory[addr + 1] << 8);
                }
                break;
            }
            case 2: { // LW: Load Word
                // std::cout << "Loading Word from : " << addr << std::endl ;
                if (addr + 3 < stack_memory.size()) {
                    data = stack_memory[addr] |
                           (stack_memory[addr + 1] << 8) |
                           (stack_memory[addr + 2] << 16) |
                           (stack_memory[addr + 3] << 24);
                }
                break;
            }
            case 6: { // LWU: Load Word Unsigned
                // std::cout << "Loading Word Unsigned from : " << addr << std::endl ;
                if (addr + 3 < stack_memory.size()) {
                    data = stack_memory[addr] |
                           (stack_memory[addr + 1] << 8) |
                           (stack_memory[addr + 2] << 16) |
                           (stack_memory[addr + 3] << 24);
                }
                break;
            }
            default:
                // Unsupported load type.
                break;
        }
        // Put the load result in MEM/WB
        next_mem_wb.writeData   = data;
        next_mem_wb.regWrite    = ex_mem.regWrite;
        next_mem_wb.instruction = ex_mem.instruction;
    }
    // If no memory operation is required (e.g. simple ALU):
    else {
        next_mem_wb.writeData   = ex_mem.aluResult;
        next_mem_wb.regWrite    = ex_mem.regWrite;
        next_mem_wb.instruction = ex_mem.instruction;
    }
    logInstructionStage(ex_mem.instruction, "MEM");

    // >>> Approach A: Flush EX/MEM afterwards for ALU or load/store instructions.
    // That way, the instruction won't stay in EX/MEM indefinitely.
    // Instruction flushNOP;
    // flushNOP.type = InstType::NOP;
    // next_ex_mem.instruction = flushNOP;
    // next_ex_mem.regWrite    = false;
    // next_ex_mem.memRead     = false;
    // next_ex_mem.memWrite    = false;
    // next_ex_mem.branch      = false;
    // next_ex_mem.aluResult   = 0;
    // next_ex_mem.rs2Val      = 0;
    // next_ex_mem.branchTarget= 0;
}




// -------------------------
// Write-Back Stage (with cycle parameter)
// -------------------------
void Processor::writeBack(int cycle) {
    if (cycle == 0) {  // First half: perform write-back.
        if (mem_wb.regWrite) {
            uint8_t rd = 0;
            switch (mem_wb.instruction.type) {
                case InstType::I_TYPE:
                    rd = mem_wb.instruction.info.i.rd;
                    break;
                case InstType::R_TYPE:
                    rd = mem_wb.instruction.info.r.rd;
                    break;
                case InstType::U_TYPE:
                    rd = mem_wb.instruction.info.u.rd;
                    break;
                case InstType::J_TYPE:
                    rd = mem_wb.instruction.info.j.rd;
                    break;
                default:
                    break;
            }
             // Only write back if the destination register is not x0.
            if (rd != 0) {
                regs[rd] = mem_wb.writeData;
            }
            // std::cout << "WriteBack: Register x" << unsigned(rd)
            //           << " updated to " << regs[rd] << std::endl;
        }
        logInstructionStage(mem_wb.instruction, "WB");
    } 

    
    // Second half: no write operations.

    // >>> Approach A: flush out MEM/WB afterwards
    // so the same instruction won't remain in MEM/WB multiple cycles.
    // Instruction nop;
    // nop.type = InstType::NOP;
    // next_mem_wb.instruction = nop;
    // next_mem_wb.regWrite    = false;
    // next_mem_wb.writeData   = 0;
}


// -------------------------
// Update Pipeline Latches and PC
// -------------------------
void Processor::updateLatches() {
    // The older pipeline latches update unconditionally:
    id_ex = next_id_ex;
    ex_mem = next_ex_mem;
    mem_wb = next_mem_wb;

    // Now handle the front end:
    if (!stallIF) {
        // Normal fetch => move next_if_id into if_id, increment PC if not branch
        if_id = next_if_id;
        if (id_ex.instruction.type != InstType::B_TYPE) {
            PC += 4; 
        }
    }
     else {
        // Freeze: do NOT update if_id or PC
        stallIF = false; // Clear for next cycle unless decode sets it again
        // std::cout << "Stalling front end, reusing same IF/ID instruction.\n";
    }
}


// -------------------------
// Run One Full Cycle
// -------------------------
void Processor::runCycle() {
    // First half (cycle = 0) for all stages.
    fetch(0);
    decode(0);
    execute(0);
    memAccess(0);
    writeBack(0); // Write-back is done in the first half.

    // Second half (cycle = 1) for all stages.
    fetch(1);
    decode(1);
    execute(1);
    // memAccess(1); // Memory access is processed through the whole cycle.
    // writeBack(1); // Write-back is done in the first half.
    
    // Commit the computed next state and update the PC.
    updateLatches();
    
    // Print the concise pipeline state.
    // printPipelineState();
    currentCycle++;
    
    // Also print detailed pipeline debug info.
    // debug_print();
}

void Processor::debug_print() {
    std::cout << "----- Debug Print: Pipeline Latches -----" << std::endl;
    
    std::cout << "IF/ID Stage: Instruction: ";
    if_id.instruction.printc_instruction();
    std::cout << std::endl << ", PC: " << if_id.pc << std::endl;
    
    
    std::cout << "ID/EX Stage: Instruction: ";
    id_ex.instruction.printc_instruction();
    std::cout << ", PC: " << id_ex.pc;
    std::cout << ", rs1Val: " << id_ex.rs1Val 
              << ", rs2Val: " << id_ex.rs2Val 
              << ", Imm: " << id_ex.imm << std::endl;
    
    std::cout << "EX/MEM Stage: Instruction: ";
    ex_mem.instruction.printc_instruction();
    std::cout << ", ALU Result: " << ex_mem.aluResult
              << ", rs2Val: " << ex_mem.rs2Val << std::endl;
    
    std::cout << "MEM/WB Stage: Instruction: ";
    mem_wb.instruction.printc_instruction();
    std::cout << ", Write Data: " << mem_wb.writeData << std::endl;
    
    std::cout << "-------------------------------------------" << std::endl;
}


// -------------------------
// Debug: Print Pipeline State
// -------------------------
void Processor::printPipelineState() {
    std::cout << "----- Pipeline State -----" << std::endl;
    std::cout << "IF/ID: "<< std::endl;
    if_id.instruction.printc_instruction();
    std::cout << std::endl;
    
    std::cout << "ID/EX: "<< std::endl;
    id_ex.instruction.printc_instruction();
    std::cout << std::endl;
    
    std::cout << "EX/MEM: "<< std::endl;
    ex_mem.instruction.printc_instruction();
    std::cout << std::endl;
    std::cout << " | ALU Result: " << ex_mem.aluResult << std::endl;
    
    std::cout << "MEM/WB: "<< std::endl;
    mem_wb.instruction.printc_instruction();
    std::cout << std::endl;
    std::cout << " | Write Data: " << mem_wb.writeData << std::endl;
}

void Processor::print_registers()
{
    std::cout << "Registers: " << std::endl;
    for (int i = 0; i < 32; i++)
    {
        std::cout << "x" << i << ": " << regs[i] << std::endl;
    }

}
void Processor::printFullPipelineLog() const {
    // Print the header row with cycle numbers.
    printPipelineLogHeader();
    // Loop through the entire pipeline log and print each instruction’s log row.
    for (size_t i = 0; i < pipelineLog.size(); ++i) {
        std::cout << "I" << (i + 1) << std::setw(5) << ":";
        for (const auto &cell : pipelineLog[i]) {
            std::cout << std::setw(10) << cell;
        }
        std::cout << std::endl;
    }
}
