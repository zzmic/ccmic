#ifndef UTILS_COMPILER_DRIVER_H
#define UTILS_COMPILER_DRIVER_H

#include <string>
#include <string_view>
#include <vector>

// Compile-time constants for compiler commands
void runCommand(std::string_view command);
void preprocess(std::string_view inputFile, std::string_view preprocessedFile);
void compileToAssembly(std::string_view preprocessedFile,
                       std::string_view assemblyFile);
void assembleToObject(std::string_view assemblyFile,
                      std::string_view objectFile);
void linkToExecutable(const std::vector<std::string> &objectFiles,
                      std::string_view executableFile);

#endif // UTILS_COMPILER_DRIVER_H
