#include "compilerDriver.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

// Run a command and check if it was successful
void runCommand(std::string_view command) {
    // Execute the given line as a shell command.
    std::cout << "Executing command: " << command << "\n";
    auto result = system(std::string{command}.c_str());

    // If the command failed, return a non-zero exit code (and shouldn't write
    // any assembly or executable files).
    if (result != 0) {
        std::stringstream msg;
        msg << "Command " << command << " failed!";
        throw std::runtime_error(msg.str());
    }
}

// Preprocess the input file and write the result to the preprocessed file.
void preprocess(std::string_view inputFile, std::string_view preprocessedFile) {
    auto command = std::string{"gcc -E -P "} + std::string{inputFile} + " -o " +
                   std::string{preprocessedFile};
    runCommand(command);
}

// Compile the preprocessed file to assembly and write the result to the
// assembly file.
void compileToAssembly(std::string_view preprocessedFile,
                       std::string_view assemblyFile) {
    auto command = std::string{"gcc -S "} + std::string{preprocessedFile} +
                   " -o " + std::string{assemblyFile};
    runCommand(command);
}

// Assemble the assembly file to an object file.
void assembleToObject(std::string_view assemblyFile,
                      std::string_view objectFile) {
    auto command = std::string{"gcc -c "} + std::string{assemblyFile} + " -o " +
                   std::string{objectFile};
    runCommand(command);
}

// Link the object files to an executable file.
void linkToExecutable(const std::vector<std::string> &objectFiles,
                      std::string_view executableFile) {
    auto command = std::string{"gcc"};
    for (const auto &objectFile : objectFiles) {
        command += " " + objectFile;
    }
    command += " -o " + std::string{executableFile};
    // Link against the C standard library.
    command += " -lc";
    runCommand(command);
}
