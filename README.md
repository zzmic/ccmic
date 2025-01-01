This project is a _work-in-progress_ implementation of a C compiler (that supports a subset of the C programming language) written in C++. The general design of the compiler is based on the book [_Writing a C Compiler_](https://nostarch.com/writing-c-compiler) by Nora Sandler. The implementation is tested against [the book's companion test suite](https://github.com/nlsandler/writing-a-c-compiler-tests.git).

To build the compiler, run the following command in the root directory of the repository:

```
# arch -x86_64 zsh # Run this command if one is running macOS on ARM and is using zsh.
make
```
