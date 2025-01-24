## ccmic

## Overview
This project is a _work-in-progress_ implementation of a C compiler (that supports a subset of the C programming language) written in C++. The general design of the compiler is based on the book [_Writing a C Compiler_](https://nostarch.com/writing-c-compiler) by Nora Sandler. The implementation is tested against [the book's companion test suite (by Nora Sandler)](https://github.com/nlsandler/writing-a-c-compiler-tests.git). On top of that, a [GoogleTest](https://github.com/google/googletest.git)-based test suite is on the way to be implemented to further validate the compiler's intermediate phases and functionalities.

The compiler is capable of processing C programs and generating corresponding assembly code, which includes the end-to-end transformation of high-level C source code into low-level assembly instructions suitable for x86-64 architecture. Key compiler phases, including lexical analysis (lexing), syntactic analysis (parsing), semantic analysis, intermediate representation (IR) generation, and (x86-64) assembly code generation and emission, are implemented. The Visitor design pattern is leveraged to traverse and manipulate the abstract syntax tree (AST) within the compiler.

## Features
### Expressions
- **Unary operations**: Supports for unary operators (`-`, `~`, `!`).
- **Binary operations**: Handles arithmetic, bitwise, assignment operators, and logical and relational operators (`+`, `-`, `*`, `/`, `%`, `&`, `|`, `=`, `&&`, `||`, `<`, `>`, `<=`, `>=`, `==`, `!=`).

### Statements
- **Local variables**: Supports declarations and usage of variables within functions and scopes.
- **If-statements and conditional expressions**: Supports control flow using `if`, `else`, and conditional (ternary) expressions (`? :`).
- **Compound statements**: Allows nesting multiple blocks using `{}`.
- **Loops**: Supports `for`, `while`, and `do-while` loops (with `break` and `continue` statements).

### Functions
- **Function definitions**: Allows defining functions with parameters and a return type.
- **Function calls**: Supports calling user-defined and standard library functions.

### File Scope and Storage Classes
- **File-scope variable declarations**: Handles global variables and their initialization.
- **Storage-class specifiers**: Supports `static` and `extern` specifiers for controlling variable visibility and linkage.

## Building the Compiler
To build the compiler, run the following command in the root directory of the repository:

```
# arch -x86_64 zsh # Run this command if one is running macOS on ARM and is using zsh.
make
```

## Usage
To compile a C source file, use the following command in the root directory of the repository:
```
bin/main [--lex] [--parse] [--validate] [--tacky] [--codegen] [-S] [-s] [-c] [-o] [--fold-constants] <sourceFile>
```
