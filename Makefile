CXX ?= clang++
UNAME_S := $(shell uname -s)

.PHONY: all debug release check-frontend check-midend format clean compiledb tidy tidy-and-fix help

ifeq ($(UNAME_S),Darwin)
  BREW_LLVM_PREFIX := $(firstword \
  $(wildcard /opt/homebrew/opt/llvm/) \
  $(wildcard /usr/local/opt/llvm/))
  BREW_LLVM_CLANG := $(BREW_LLVM_PREFIX)bin/clang++
  ifneq ($(wildcard $(BREW_LLVM_CLANG)),)
    ifneq (,$(filter $(CXX),c++ clang++))
      CXX := $(BREW_LLVM_CLANG)
    endif
  endif
endif
STDFLAGS = -std=c++23
STDLIBFLAGS :=
ifeq ($(UNAME_S),Darwin)
  ifneq ($(findstring clang,$(notdir $(CXX))),)
    STDLIBFLAGS := -stdlib=libc++
  endif
endif
SYSROOT :=
SYSROOTFLAGS :=
ifeq ($(UNAME_S),Darwin)
  SYSROOT := $(shell xcrun --show-sdk-path 2>/dev/null)
  ifneq ($(strip $(SYSROOT)),)
    SYSROOTFLAGS := -isysroot $(SYSROOT)
  endif
endif
LDFLAGS = $(STDLIBFLAGS) $(if $(filter -stdlib=libc++,$(STDLIBFLAGS)),-L$(BREW_LLVM_PREFIX)lib/c++)
# References:
# 1. <https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md#gcc--clang>>.
# 2. <https://clang.llvm.org/docs/DiagnosticsReference.html>.
WARNFLAGS = -Werror -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic \
  -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic \
  -Wconversion -Wsign-conversion -Wmisleading-indentation -Wnull-dereference \
  -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough \
  -Wfor-loop-analysis -Wrange-loop-analysis
CXXFLAGS = $(STDFLAGS) $(WARNFLAGS) $(STDLIBFLAGS) $(SYSROOTFLAGS) -O2
LDLIBS =
SAN_FLAGS = -fsanitize=address,undefined,vptr,leak -fno-omit-frame-pointer -fno-sanitize-recover=all

SRC_DIR = src
FRONTEND_DIR = $(SRC_DIR)/frontend
MIDEND_DIR = $(SRC_DIR)/midend
BACKEND_DIR = $(SRC_DIR)/backend
UTILS_DIR = $(SRC_DIR)/utils
BIN_DIR = bin

# All source files in the project.
SOURCES = $(wildcard $(FRONTEND_DIR)/*.cpp) \
  $(wildcard $(MIDEND_DIR)/*.cpp) \
  $(wildcard $(BACKEND_DIR)/*.cpp) \
  $(wildcard $(UTILS_DIR)/*.cpp) \
  $(wildcard $(SRC_DIR)/*.cpp)

# Header files corresponding to all source files.
HEADERS = $(wildcard $(FRONTEND_DIR)/*.h) \
  $(wildcard $(MIDEND_DIR)/*.h) \
  $(wildcard $(BACKEND_DIR)/*.h) \
  $(wildcard $(UTILS_DIR)/*.h) \
  $(wildcard $(SRC_DIR)/*.h)

# Object files corresponding to all source files.
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SOURCES))

# Frontend-only compilation check target.
FRONTEND_SOURCES_ALL = $(wildcard $(FRONTEND_DIR)/*.cpp)
FRONTEND_SOURCES = $(FRONTEND_SOURCES_ALL)
FRONTEND_HEADERS = $(wildcard $(FRONTEND_DIR)/*.h)
FRONTEND_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(FRONTEND_SOURCES))

# Frontend + midend compilation check target.
MIDEND_SOURCES = $(wildcard $(FRONTEND_DIR)/*.cpp) \
  $(wildcard $(MIDEND_DIR)/*.cpp)
MIDEND_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(MIDEND_SOURCES))

# Main executable path.
EXECUTABLE = $(BIN_DIR)/main

# Default target to build the project.
all: $(BIN_DIR) $(EXECUTABLE)

# Create the binary directory if it doesn't exist.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile each source file into an object file in the binary directory, preserving the directory structure.
# The `|` symbol indicates that the `$(BIN_DIR)` directory must exist before the target can be built,
# but changes to this directory won't trigger a rebuild of the object files.
# Reference: <https://www.gnu.org/software/make/manual/html_node/Prerequisite-Types.html>.
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BIN_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Generate dependency files for each source file.
CXXFLAGS += -MMD -MP
DEPS = $(OBJECTS:.o=.d)
-include $(DEPS)

# Link the object files to create the executable.
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

# Debug target with most optimizations disabled, full debug symbols, sanitizers, and runtime hardening.
debug: CXXFLAGS += -O0 -g $(SAN_FLAGS) -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG
debug: LDFLAGS  += $(SAN_FLAGS)
debug: all

# Release target with optimizations, hardening flags, and minimal debug info for profiling/stack traces.
release: CXXFLAGS += -O3 -DNDEBUG -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fvisibility=hidden -fno-omit-frame-pointer -gline-tables-only
release: all

# Frontend-only compilation check target for debugging purposes.
# This compiles only the frontend sources to verify they compile independently.
check-frontend: $(BIN_DIR) $(FRONTEND_OBJECTS)
	@echo "Frontend compilation check passed!"

# Frontend + midend compilation check target for debugging purposes.
# This compiles the frontend and midend sources (no linking).
check-midend: $(BIN_DIR) $(MIDEND_OBJECTS)
	@echo "Frontend + midend compilation check passed!"

# Format C++ header and source files using `clang-format` (that obeys `.clang-format`'s configuration).
format:
	clang-format -i $(SOURCES) $(HEADERS)

# Clean build artifacts.
clean:
	rm -rfv $(BIN_DIR)/*
	@find $(BIN_DIR) -name '*.d' -delete 2>/dev/null || true

# Generate `compile_commands.json` for tooling support (e.g., `clang-tidy`, `clangd`).
compiledb:
	@which compiledb >/dev/null || (echo "Error: `compiledb` not found. Please install it to generate `compile_commands.json`." >&2; exit 1)
	compiledb make

# Run `clang-tidy` checks on the header and source files, obeying the configuration specified in `.clang-tidy`.
tidy:
	@which clang-tidy >/dev/null || (echo "Error: `clang-tidy` not found. Please install it to run tidy checks." >&2; exit 1)
	run-clang-tidy -p .

# Run `clang-tidy` checks and apply automatic fixes (if available) to the header and source files, obeying the configuration specified in `.clang-tidy`.
tidy-and-fix:
	@which clang-tidy >/dev/null || (echo "Error: `clang-tidy` not found. Please install it to run tidy checks." >&2; exit 1)
	run-clang-tidy -fix -p .

help:
	@echo 'Usage: make <target>'
	@echo
	@echo 'Targets:'
	@printf '  %-15s %s\n' 'all' 'Build the project (default target).'
	@printf '  %-15s %s\n' 'debug' 'Build with most optimizations disabled, full debug symbols, sanitizers, and runtime hardening for debugging.'
	@printf '  %-15s %s\n' 'release' 'Build with optimizations, hardening flags, and minimal debug info for profiling/stack traces for release.'
	@printf '  %-15s %s\n' 'check-frontend' 'Compile frontend sources only (no linking).'
	@printf '  %-15s %s\n' 'check-midend' 'Compile frontend + midend sources (no linking).'
	@printf '  %-15s %s\n' 'format' 'Format C++ header and source files using `clang-format`.'
	@printf '  %-15s %s\n' 'clean' 'Remove build artifacts.'
	@printf '  %-15s %s\n' 'compiledb' 'Generate `compile_commands.json` for tooling support.'
	@printf '  %-15s %s\n' 'tidy' 'Run `clang-tidy` checks on the header and source files.'
	@printf '  %-15s %s\n' 'tidy-and-fix' 'Run `clang-tidy` checks and apply automatic fixes (if available) to the header and source files.'
	@printf '  %-15s %s\n' 'help' 'Show this help message.'
