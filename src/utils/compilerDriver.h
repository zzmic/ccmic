#ifndef UTILS_COMPILER_DRIVER_H
#define UTILS_COMPILER_DRIVER_H

#include <string>
#include <string_view>
#include <vector>

/**
 * Run a command and check if it was successful.
 *
 * @param command The command to run.
 */
void runCommand(std::string_view command);

/**
 * Preprocess the input file and write the result to the preprocessed file.
 *
 * @param inputFile The input source file.
 */
void preprocess(std::string_view inputFile, std::string_view preprocessedFile);

/**
 * Compile the preprocessed file to assembly and write the result to the
 * assembly file.
 *
 * @param preprocessedFile The preprocessed source file.
 * @param assemblyFile The output assembly file.
 */
void compileToAssembly(std::string_view preprocessedFile,
                       std::string_view assemblyFile);

/**
 * Assemble the assembly file to an object file.
 *
 * @param assemblyFile The input assembly file.
 * @param objectFile The output object file.
 */
void assembleToObject(std::string_view assemblyFile,
                      std::string_view objectFile);

/**
 * Link the object files to an executable file.
 *
 * @param objectFiles The list of object files to link.
 * @param executableFile The output executable file.
 */
void linkToExecutable(const std::vector<std::string> &objectFiles,
                      std::string_view executableFile);

#endif // UTILS_COMPILER_DRIVER_H
