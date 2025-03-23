# Makefile for the Processor_COL216 project

# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++11 -Wall -g -Iheader

# Source files for main simulation (excluding test_instruction.cpp)
SRCS  = src/ALU.cpp \
        src/ControlUnit.cpp \
        src/Instruction.cpp \
        src/PipelineStage.cpp \
        src/Processor.cpp \
        src/Utils.cpp \
        src/main.cpp

# Object files automatically inferred
OBJS  = $(SRCS:.cpp=.o)

# Default target: build both non-forwarding and forwarding executables
all: noforward forward

# Non-forwarding executable
noforward: $(OBJS)
	$(CXX) $(CXXFLAGS) -o noforward $(OBJS)

# Forwarding executable (same object files, but different runtime mode)
forward: $(OBJS)
	$(CXX) $(CXXFLAGS) -o forward $(OBJS)

# Optional target to build and run test_instruction
# (Standalone test for instruction decoding logic.)
test_instruction: src/test_instruction.cpp src/Instruction.cpp
	$(CXX) $(CXXFLAGS) -c src/Instruction.cpp -o Instruction.o
	$(CXX) $(CXXFLAGS) -c src/test_instruction.cpp -o test_instruction.o
	$(CXX) $(CXXFLAGS) -o test_instruction test_instruction.o Instruction.o

# Generic rule to compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f src/*.o *.o noforward forward test_instruction
