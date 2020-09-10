CXX       := g++
CXX_FLAGS := -Wall -Wextra

BIN        := bin
SRC        := src
INCLUDE    := include
EXECUTABLE := main
LIBRARIES  :=

all: $(BIN)/$(EXECUTABLE)

run: clean all
	@echo "Executing..."
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	@echo "Building..."
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	@echo "Clearing..."
	-rm $(BIN)/*
