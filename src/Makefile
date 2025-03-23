CXX = g++
CXXFLAGS = -std=c++11 -Wall

SRC = ALU.cpp ControlUnit.cpp Instruction.cpp PipelineStage.cpp Processor.cpp Utils.cpp main.cpp
OBJ = $(SRC:.cpp=.o)

# Default target: build non-forwarding version.
noforward: $(OBJ)
	$(CXX) $(CXXFLAGS) -DFORWARDING_DISABLED -o noforward $(OBJ)

# Build forwarding version.
forward: $(OBJ)
	$(CXX) $(CXXFLAGS) -DFORWARDING_ENABLED -o forward $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) noforward forward
