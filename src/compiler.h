

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

void runCommand(const std::string &command);
void preprocess(const std::string &inputFile,
                const std::string &preprocessedFile);
void compileToAssembly(const std::string &preprocessedFile,
                       const std::string &assemblyFile);
void assembleAndLink(const std::string &assemblyFile,
                     const std::string &outputFile);

#endif // COMPILER_H
