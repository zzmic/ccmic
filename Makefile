# Compiler and flags.
CXX = clang++
CXXFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow -Wnull-dereference -std=c++23 -stdlib=libc++

# Directories.
SRC_DIR = src
FRONTEND_DIR = $(SRC_DIR)/frontend
MIDEND_DIR = $(SRC_DIR)/midend
BACKEND_DIR = $(SRC_DIR)/backend
UTILS_DIR = $(SRC_DIR)/utils
BIN_DIR = bin

# Source files.
SOURCES = $(wildcard $(FRONTEND_DIR)/*.cpp) \
		  $(wildcard $(MIDEND_DIR)/*.cpp) \
		  $(wildcard $(BACKEND_DIR)/*.cpp) \
		  $(wildcard $(UTILS_DIR)/*.cpp) \
		  $(wildcard $(SRC_DIR)/*.cpp)
# Header files.
HEADERS = $(wildcard $(FRONTEND_DIR)/*.h) \
		  $(wildcard $(MIDEND_DIR)/*.h) \
		  $(wildcard $(BACKEND_DIR)/*.h) \
		  $(wildcard $(UTILS_DIR)/*.h) \
		  $(wildcard $(SRC_DIR)/*.h)
# Object files.
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES))
# Executable file.
EXECUTABLE = $(BIN_DIR)/main

# Default target.
all: $(BIN_DIR) $(EXECUTABLE)

# Create the `bin` directory if it does not exist.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile the source files to object files.
# The `|` symbol indicates that the `$(BIN_DIR)` directory must exist
# before the target can be built,
# but changes to this directory won't trigger a rebuild of the object files.
# https://www.gnu.org/software/make/manual/html_node/Prerequisite-type.html.
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BIN_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link the object files to create the executable.
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ -stdlib=libc++

# Debug target.
debug: CXXFLAGS += -g
debug: all

# Release target with optimizations.
# release: CXXFLAGS += -O3 -DNDEBUG
# release: all

# Format the source files and the header files.
format:
	clang-format -i $(SOURCES) $(HEADERS)

# Clean up the object files and the executable.
clean:
	rm -rf $(BIN_DIR)/*

# .PHONY: all debug release clean
.PHONY: all debug clean
