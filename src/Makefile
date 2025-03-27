# Makefile for the Processor_COL216 project (inside src directory)

# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++11 -Wall -g

# Source files (all are now in the current directory)
SRCS  = ALU.cpp \
        ControlUnit.cpp \
        Instruction.cpp \
        PipelineStage.cpp \
        Processor.cpp \
        Utils.cpp \
        main.cpp

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
	rm -f *.o noforward forward test_instruction
