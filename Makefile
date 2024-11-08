# Compiler and flags.
CXX = clang++
CXXFLAGS = -Wall -Wextra -std=c++20

# Directories.
SRC_DIR = src
BIN_DIR = bin

# Source files.
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Object files.
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES))
# Executable file.
EXECUTABLE = $(BIN_DIR)/main

# Default target.
all: $(EXECUTABLE)

# Create the `bin` directory if it does not exist.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Link the object files to create the executable.
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Compile the source files to object files.
# The `|` symbol indicates that the `$(BIN_DIR)` directory must exist
# before the target can be built,
# but changes to this directory won't trigger a rebuild of the object files.
# https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html.
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the object files and the executable.
clean:
	rm -f $(BIN_DIR)/*.o $(EXECUTABLE)

.PHONY: all clean
