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
 * @param inputFileName The input source file.
 * @param preprocessedFileName The output preprocessed file.
 */
void preprocess(std::string_view inputFileName,
                std::string_view preprocessedFileName);

/**
 * Compile the preprocessed file to assembly and write the result to the
 * assembly file.
 *
 * @param preprocessedFileName The preprocessed source file.
 * @param assemblyFileName The output assembly file.
 */
void compileToAssembly(std::string_view preprocessedFileName,
                       std::string_view assemblyFileName);

/**
 * Assemble the assembly file to an object file.
 *
 * @param assemblyFileName The input assembly file.
 * @param objectFileName The output object file.
 */
void assembleToObject(std::string_view assemblyFileName,
                      std::string_view objectFileName);

/**
 * Link the object files to an executable file.
 *
 * @param objectFileNames The list of object files to link.
 * @param executableFileName The output executable file.
 */
void linkToExecutable(const std::vector<std::string> &objectFileNames,
                      std::string_view executableFileName);

#endif // UTILS_COMPILER_DRIVER_H
