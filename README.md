# Processor_COL216

## Project Overview

This project is an object-oriented C++ simulator for a **5-stage RISC-V pipelined processor**. The simulator supports two variants – one **with forwarding** and one **without forwarding** – and implements all core stages: Instruction Fetch (IF), Instruction Decode (ID), Execute (EX), Memory Access (MEM), and Write Back (WB).

## Key Features

- **Dual Variants:** Supports both a pipelined processor with forwarding and one without. Forwarding paths are used to resolve data hazards when enabled.
- **Comprehensive Instruction Support:** Test cases and implementations cover a wide range of instructions including:
  - **Arithmetic:** `add`, `addi`, `sub`, `mul`, `div`
  - **Shifts:** `slli`, `sll`, `srli`, `srl`, `srai`, `sra`
  - **Memory Operations:** `lw`, `sw`, `lb`, `sb`, and load/store halfword variants.
  - **Control Flow:** Branching (decision taken in the ID stage) and jumps.
- **Pipeline Stages & Latches:** Implements 5 pipeline stages using five latches (IF/ID, ID/EX, EX/MEM, MEM/WB, and next state registers). In each cycle, every stage takes input from the previous latch and produces output for the next, accurately simulating pipelined behavior.
- **Stall and No-Op Insertion:** The design includes hazard detection logic that inserts NoOps (NOPs) and stalls the pipeline when required. This ensures registers are updated correctly after every operation.
- **Memory Simulation:** Memory is simulated to support load and store instructions. The memory model enforces that `x0` is hardwired to 0.
- **Error Handling:** If an unsupported or incorrect machine code is encountered, an error is raised.
- **Pipeline Logging:** A table is printed that details the pipeline stages for each instruction over the cycles. This table has been cross-verified with the RIPES simulator’s output for correctness.

## Data Structures & Design Decisions

- **Instruction Representation:**  
  Each instruction is represented by an `Instruction` object that decodes a 32-bit machine code into its constituent fields (opcode, funct3, funct7, immediate, etc.). This abstraction simplifies control signal generation and pipeline logging.

- **Control Unit & ALU:**  
  The `ControlUnit` class generates control signals based on the decoded instruction type. The `ALU` class supports various operations (arithmetic and shifts) and ensures proper handling of shift amounts using the lower 5 bits of the second operand.

- **Pipeline Latches:**  
  The simulator employs five pipeline latches (IF/ID, ID/EX, EX/MEM, MEM/WB, and the next state registers). In each cycle, a stage takes input from the previous latch and writes its result to the next latch, simulating the flow of instructions through the pipeline.

- **Forwarding & Hazard Detection:**  
  Hazard detection logic is implemented in the ID stage. When forwarding is enabled, the EX stage overrides register values using data from later pipeline stages to resolve hazards. This minimizes the need for stalls.

- **Register File & Memory:**  
  A vector of 32 registers is maintained, with **`x0` hardwired to 0**. Memory is modeled as a vector to simulate load and store instructions (such as `lw`, `sw`, `lb`, `sb`).

## Test Cases & Verification

Test cases have been implemented for all major instruction types:
- **Arithmetic & Logical Operations:** `add`, `addi`, `sub`, `mul`, `div`, and shift instructions (`slli`, `sll`, `srli`, `srl`, `srai`, `sra`).
- **Memory Operations:** Test cases for `lw`, `sw`, `lb`, `sb` ensure correct simulation of memory access.
- **Control Flow:** Branch and jump instructions have been verified; branch decisions are taken in the ID stage.
- **Pipeline Diagrams:** The simulator prints a detailed pipeline diagram (with cycle numbers and stage information). These diagrams have been compared with the RIPES simulator's output to ensure correctness.

## Conclusion

The simulator has been thoroughly tested. It accurately updates registers (with `x0` always remaining 0) and simulates memory for load/store instructions. With five pipeline latches, hazard detection, and proper insertion of NOPs, the simulator models both forwarding and non-forwarding scenarios effectively. All test cases have been validated against RIPES simulation output.

## Sources & Acknowledgements

- **Course Materials & Assignment PDF:** The design is based on the assignment specifications and provided example functions.
- **RIPES Simulator:** Used for cross-verification of pipeline diagrams and test cases.
- **ChatGPT:** Consulted for clarifying concepts and design decisions during the development process.

## How to Run the Simulator

- **Compilation:** Navigate to the `src/` directory and run `make` to build both the `noforward` and `forward` executables.
- **Execution:** 
  - Non-forwarding: `./noforward ../inputfiles/filename.txt cycleCount`
  - Forwarding: `./forward ../inputfiles/filename.txt cycleCount`
- **Output:** The simulator will write the output in an **output.txt** file with detailed information on which instruction is in which stage in which cycle. Various data from the pipeline latches is captured each cycle, and a proper table along with a simplified output (as specified in the assignment PDF) is written to output.txt.
- **Note:** To get the same output as that of our test cases when you run it as told above you would need to modify the main.cpp to enable the print functions.

