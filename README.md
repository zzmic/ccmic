# ccmic

## Overview
This project is a C++ implementation of a C compiler for a non-trivial subset of the C programming language, adhering to the C17 standard. The compiler's design is based on the principles and practices outlined in [_Writing a C Compiler_ by Nora Sandler](https://nostarch.com/writing-c-compiler). It is continuously developed and tested against the [the book's companion test suite (hosted by Nora Sandler)](https://github.com/nlsandler/writing-a-c-compiler-tests.git). As new features and optimizations are integrated into the codebase, the compiler's capabilities continue to expand modularly.

The compiler transforms C source code into x86-64 assembly through a multi-stage pipeline:

- **Lexing**: Regex-based tokenization of C source.
- **Parsing**: Recursive descent parsing with precedence climbing for AST construction.
   - Leverages the [Visitor design pattern](https://en.wikipedia.org/wiki/Visitor_pattern) for AST traversal.
- **Semantic Analysis**: Type checking, symbol resolution, and loop labeling.
- **IR Generation**: AST lowering to a custom intermediate representation (IR).
- **Code Generation**: IR-to-assembly translation, stack allocation, and fixup passes.
- **Assembly Emission**: Final x86-64 assembly output.

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

### Expressions
- **Unary operations**: `-`, `~`, `!`.
- **Binary operations**: Arithmetic (`+`, `-`, `*`, `/`, `%`), bitwise (`&`, `|`), assignment (`=`), logical (`&&`, `||`), relational (`<`, `>`, `<=`, `>=`, `==`, `!=`).

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
The codebase is organized into modular components:
- **src/frontend/**: Lexer, parser, AST, and semantic analysis.
- **src/midend/**: IR generation (and optimization passes to be implemented).
- **src/backend/**: Assembly generation, stack allocation, and fixup passes.
- **src/utils/**: Pipeline orchestration and pretty-printers for debugging.

## Building and Usage

### Building the Compiler
```
# arch -x86_64 zsh # Run this command if one is running macOS on ARM and is using zsh.
make
```

### Compiling C Programs
```
bin/main [--lex] [--parse] [--validate] [--tacky] [--codegen] [-S] [-s] [-c] [-o] <sourceFile>
```

### Command-Line Flags
- **Pipeline control**: `--lex` (lexical analysis), `--parse` (syntactic analysis), `--validate` (semantic analysis), `--tacky` (IR generation), `--codegen` (code generation).
- **Output options**: `-S`/`-s` (assembly emission), `-c` (object file emission).
- **Optimizations** (to be implemented): `--fold-constants`, `--propagate-copies`, `--eliminate-unreachable-code`, `--eliminate-dead-stores`, `--optimize`.

## Development and Extensibility
- **Adding language features**: Extend frontend classes and corresponding IR representations in `src/frontend/` and `src/midend/`.
- **Adding optimizations**: Implement new passes in `src/midend/` following the existing `OptimizationPass` pattern.
- **Debugging**: Use built-in pretty-printers for IR and assembly inspection in `src/utils/`.
- **Testing**: Run against the companion test suite linked above.
