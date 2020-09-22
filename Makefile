CXX       := g++
CXX_FLAGS := -Wall -Wextra

BIN        := bin
SRC        := src
INCLUDE    := include
EXECUTABLE := main
LIBRARIES  :=

all: $(BIN)/$(EXECUTABLE)

run: clean all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	-rm -f $(BIN)/*
