# Compiler.
CXX = /usr/bin/clang++

# Common (compiler) flags.
STDFLAGS = -std=c++23
STDLIBFLAGS = -stdlib=libc++
WARNFLAGS = -Werror -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnull-dereference \
  -Wsign-conversion -Wimplicit-fallthrough -Wrange-loop-analysis
CXXFLAGS = $(STDFLAGS) $(WARNFLAGS) $(STDLIBFLAGS)
LDFLAGS = $(STDLIBFLAGS)
LDLIBS =

# Sanitizer and hardening flags (for debug builds).
SAN_FLAGS = -fsanitize=address,undefined,vptr -fno-omit-frame-pointer -fno-sanitize-recover=all

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
.PHONY: all clean debug format release

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

# Automatic dependency generation.
CXXFLAGS += -MMD -MP
DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

# Link the object files to create the executable.
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

# Debug target.
debug: CXXFLAGS += -g -O0 $(SAN_FLAGS) -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
debug: LDFLAGS  += $(SAN_FLAGS)
debug: all

# Release target with optimizations and without debug symbols.
release: CXXFLAGS += -O3 -DNDEBUG
release: all

# Format the source files and the header files.
format:
	clang-format -i $(SOURCES) $(HEADERS)

# Clean up the object files, executable, and dependency files.
clean:
	rm -rf $(BIN_DIR)/* $(BIN_DIR)/**/*.d
