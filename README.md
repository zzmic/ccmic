# ccmic

## Overview

**`ccmic`** is a C++ implementation of a C compiler for a non-trivial subset of the C programming language, adhering to the C17 standard. The compiler's design is based on the principles and practices outlined in [_Writing a C Compiler_ by Nora Sandler](https://nostarch.com/writing-c-compiler). It is continuously developed and tested against the [the book's companion test suite (hosted by Nora Sandler)](https://github.com/nlsandler/writing-a-c-compiler-tests.git) (also included in the repository as a submodule (`tests/`)). As new features and optimizations are integrated into the codebase, the compiler's capabilities continue to expand modularly.

The compiler transforms C source code into X86-64 assembly through a multi-stage pipeline:

- **Lexing (Lexical Analysis)**: Regex-based tokenization of C source.
- **Parsing (Syntactic Analysis)**: Recursive descent parsing with precedence climbing for abstract syntax tree (AST) construction.
  - Leverages [the Visitor design pattern](https://en.wikipedia.org/wiki/Visitor_pattern) for AST traversal(s), where `AST` nodes accept a `Visitor` interface (defined in [`src/frontend/visitor.h`](https://github.com/zzmic/ccmic/blob/main/src/frontend/visitor.h)), in which the design separates algorithms (e.g., pretty-printing, semantic analysis, IR generation) from the object structure, enabling the addition of new operations without modifying the AST classes.
- **Semantic Analysis**: Type checking, symbol resolution, and loop labeling for AST validation.
- **IR Generation**: AST lowering to a custom intermediate representation (IR).
- **Code Generation (Assembly Generation)**: IR-to-assembly translation, stack allocation, and fixup passes for X86-64.
- **Assembly Emission**: Final X86-64 assembly output ready for assembling and linking to an executable.

```
╭────────────────────────╮
│         Lexing         │
│   (Lexical Analysis)   │
╰────────────────────────╯
             │
             ▼
╭──────────────────────────╮
│         Parsing          │
│   (Syntactic Analysis)   │
╰──────────────────────────╯
             │
             ▼
╭───────────────────────╮
│   Semantic Analysis   │
╰───────────────────────╯
             │
             ▼
╭───────────────────────╮
│     IR Generation     │
╰───────────────────────╯
             │
             ▼
╭───────────────────────────╮
│      Code Generation      │
│   (Assembly Generation)   │
╰───────────────────────────╯
             │
             ▼
╭───────────────────────╮
│   Assembly Emission   │
╰───────────────────────╯
```

## Supported Language Features

### Data Types

- **Signed integers**: `int` (32-bit) and `long` (64-bit).
- **Unsigned integers**: `unsigned int` (32-bit) and `unsigned long` (64-bit).

### Expressions

- **Unary operations**: `-`, `~`, and `!`.
- **Binary operations**: Arithmetic (`+`, `-`, `*`, `/`, `%`), bitwise (`&`, `|`), assignment (`=`), logical (`&&`, `||`), and relational (`<`, `>`, `<=`, `>=`, `==`, `!=`).
- **Type conversions**: Explicit casts and implicit conversions following the usual arithmetic conversion rules.

### Statements

- **Local variables**: Declarations and usage within functions and scopes.
- **Control flow**: `if`/`else` statements and conditional expressions (`? :`).
- **Compound statements**: Nested blocks using `{}`.
- **Loops**: `for`, `while`, and `do-while` loops with `break` and `continue`.

### Functions

- **Function definitions**: Parameters and return types.
- **Function calls**: User-defined and standard library functions.

### File Scope and Storage Classes

- **Global variables**: File-scope declarations and initialization.
- **Storage specifiers**: `static` and `extern` for visibility and linkage control.

## Project Structure

The implementation is organized into several key directories and file(s):

- **[src/frontend/](https://github.com/zzmic/ccmic/tree/main/src/frontend)**: Lexer, parser, AST, and semantic analysis.
- **[src/midend/](https://github.com/zzmic/ccmic/tree/main/src/midend)**: IR generation (and optimization passes to be implemented).
- **[src/backend/](https://github.com/zzmic/ccmic/tree/main/src/backend)**: Assembly generation, stack allocation, and fixup passes.
- **[src/utils/](https://github.com/zzmic/ccmic/tree/main/src/utils)**: Pipeline orchestration, including assembly emission, and pretty-printers for debugging.
- **[src/main.cpp](https://github.com/zzmic/ccmic/blob/main/src/main.cpp)**: Entry point of the compiler, orchestrating the compilation pipeline based on command-line arguments.

```bash
➜  ccmic git:(main) tree src
src
├── backend
│   ├── assembly.cpp
│   ├── assembly.h
│   ├── assemblyGenerator.cpp
│   ├── assemblyGenerator.h
│   ├── backendSymbolTable.cpp
│   ├── backendSymbolTable.h
│   ├── fixupPass.cpp
│   ├── fixupPass.h
│   ├── pseudoToStackPass.cpp
│   └── pseudoToStackPass.h
├── frontend
│   ├── ast.h
│   ├── block.cpp
│   ├── block.h
│   ├── blockItem.cpp
│   ├── blockItem.h
│   ├── constant.cpp
│   ├── constant.h
│   ├── declaration.cpp
│   ├── declaration.h
│   ├── expression.cpp
│   ├── expression.h
│   ├── forInit.cpp
│   ├── forInit.h
│   ├── frontendSymbolTable.h
│   ├── function.cpp
│   ├── function.h
│   ├── lexer.cpp
│   ├── lexer.h
│   ├── operator.cpp
│   ├── operator.h
│   ├── parser.cpp
│   ├── parser.h
│   ├── printVisitor.cpp
│   ├── printVisitor.h
│   ├── program.cpp
│   ├── program.h
│   ├── semanticAnalysisPasses.cpp
│   ├── semanticAnalysisPasses.h
│   ├── statement.cpp
│   ├── statement.h
│   ├── storageClass.cpp
│   ├── storageClass.h
│   ├── type.cpp
│   ├── type.h
│   └── visitor.h
├── main.cpp
├── midend
│   ├── ir.cpp
│   ├── ir.h
│   ├── irGenerator.cpp
│   ├── irGenerator.h
│   ├── irOptimizationPasses.cpp
│   └── irOptimizationPasses.h
└── utils
    ├── compilerDriver.cpp
    ├── compilerDriver.h
    ├── constants.h
    ├── pipelineStagesExecutors.cpp
    ├── pipelineStagesExecutors.h
    ├── prettyPrinters.cpp
    └── prettyPrinters.h
```

## Building and Usage

### Cloning the Repository

```bash
git clone --recurse-submodules https://github.com/zzmic/ccmic.git
```

### Installing Dependencies

- **[Clang](https://clang.llvm.org)** that supports C++23 (or above) for building the compiler.
- **[GCC](https://gcc.gnu.org)** that supports X86-64 and C17 for preprocessing, assembling, and linking.

### Building the Compiler

```bash
# Remove any previous build artifacts if necessary.
make clean
# Build the compiler with the number of available CPU cores for parallel compilation.
make -j$(nproc)
```

## Compiling C Programs

### Command-Line Usage

```bash
bin/main [--lex] [--parse] [--validate] [--tacky] [--codegen] [-S] [-s] [-c] [-o <outputFile>] <sourceFile>
```

### Command-Line Flags

- **Pipeline control**: `--lex` (lexical analysis), `--parse` (syntactic analysis), `--validate` (semantic analysis), `--tacky` (IR generation), and `--codegen` (code generation).
- **Output options**: `-S` or `-s` (assembly emission), `-c` (object file emission), and `-o <outputFile>` (specify output file, default to the program name (i.e., the base name of the source file)).
- **Optimizations** (_to be implemented_): `--fold-constants` (constant folding), `--eliminate-unreachable-code` (dead code elimination), `--propagate-copies` (copy propagation), `--eliminate-dead-stores` (dead store elimination), and `--optimize` (enable all optimizations).

## Generating JSON Compilation Database Files

To generate a `compile_commands.json` file for use with tools that support the [JSON compilation database format](https://clang.llvm.org/docs/JSONCompilationDatabase.html), run the following command (assuming that [compiledb](https://github.com/nickdiego/compiledb) is installed):

```bash
compiledb make -j$(nproc)
```

or

```bash
make -j$(nproc) compiledb
```

## Static Analysis with Clang-Tidy

To perform static analysis on the codebase using [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/), run the following command (assuming that the LLVM toolchain is installed) with the compilation database generated in the previous step/section:

```bash
run-clang-tidy -j$(nproc) -p .
```

or

```bash
make -j$(nproc) tidy
```

The checks and options for Clang-Tidy are configured (and can be further customized) in [`.clang-tidy`](https://github.com/zzmic/ccmic/blob/main/.clang-tidy).

## Development and Extensibility

- **Adding language features**: Expand the frontend in [`src/frontend/`](https://github.com/zzmic/ccmic/tree/main/src/frontend) by modifying the lexer, parser, and AST nodes, as well as updating semantic analysis.
- **Adding optimizations**: Implement new optimization passes in [`src/midend/`](https://github.com/zzmic/ccmic/tree/main/src/midend) by following the existing `IR::OptimizationPass` pattern.
- **Extending code generation**: Modify or add new code generation strategies in [`src/backend/`](https://github.com/zzmic/ccmic/tree/main/src/backend).
- **Debugging**: Leverage the implemented pretty-printers for IR and assembly inspection in [`src/utils/`](https://github.com/zzmic/ccmic/tree/main/src/utils) (and [`gdb`](https://www.sourceware.org/gdb/)/[`lldb`](https://lldb.llvm.org/)).
- **Testing**: Run tests using the companion test suite linked in the overview section.

## Disclaimer

This project is _not_ an orthodox implementation or artifact from the referenced literature. Therefore, I should be accountable for any errors and flaws in the implementation and documentation, not the original authors of the sources.
