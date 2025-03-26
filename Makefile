# Makefile for the Processor_COL216 project

# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++11 -Wall -g -Iheader

# Source files for main simulation
SRCS  = src/ALU.cpp \
        src/ControlUnit.cpp \
        src/Instruction.cpp \
        src/PipelineStage.cpp \
        src/Processor.cpp \
        src/Utils.cpp \
        src/main.cpp

# Object files for non-forwarding build
OBJS  = $(SRCS:.cpp=.o)

# Object files for forwarding build (compiled with -DFORWARDING)
OBJS_FORWARD = $(SRCS:.cpp=.forward.o)

# Default target: build both non-forwarding and forwarding executables
all: noforward forward

# Non-forwarding executable (no extra flag)
noforward: $(OBJS)
	$(CXX) $(CXXFLAGS) -o noforward $(OBJS)

# Forwarding executable: add the -DFORWARDING flag.
forward: $(OBJS_FORWARD)
	$(CXX) $(CXXFLAGS) -DFORWARDING -o forward $(OBJS_FORWARD)

# Generic rule to compile .cpp to .o (non-forwarding)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generic rule to compile .cpp to .forward.o (forwarding)
%.forward.o: %.cpp
	$(CXX) $(CXXFLAGS) -DFORWARDING -c $< -o $@

# Clean up object files and executables
clean:
	rm -f src/*.o *.o noforward forward test_instruction
