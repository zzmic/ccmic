#ifndef UTILS_COMPILER_DRIVER_H
#define UTILS_COMPILER_DRIVER_H

#include <string>
#include <vector>

void runCommand(const std::string &command);
void preprocess(const std::string &inputFile,
                const std::string &preprocessedFile);
void compileToAssembly(const std::string &preprocessedFile,
                       const std::string &assemblyFile);
void assembleToObject(const std::string &assemblyFile,
                      const std::string &objectFile);
void linkToExecutable(const std::vector<std::string> &objectFiles,
                      const std::string &executableFile);

#endif // UTILS_COMPILER_DRIVER_H
