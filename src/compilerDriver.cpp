#include "compilerDriver.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>

// Run a command and check if it was successful
void runCommand(const std::string &command) {
    // Execute the given line as a shell command.
    std::cout << "Executing command: " << command << "\n";
    int result = system(command.c_str());

    // If the command failed, return a non-zero exit code (and shouldn't write
    // any assembly or executable files).
    if (result != 0) {
        std::stringstream msg;
        msg << "Command " << command << " failed!";
        throw std::runtime_error(msg.str());
    }
}

// Preprocess the input file and write the result to the preprocessed file.
void preprocess(const std::string &inputFile,
                const std::string &preprocessedFile) {
    std::string command = "gcc -E " + inputFile + " -o " + preprocessedFile;
    runCommand(command);
}

// Compile the preprocessed file to assembly and write the result to the
// assembly file.
void compileToAssembly(const std::string &preprocessedFile,
                       const std::string &assemblyFile) {
    std::string command = "gcc -S " + preprocessedFile + " -o " + assemblyFile;
    runCommand(command);
}

// Assemble the assembly file to an object file.
void assembleToObject(const std::string &assemblyFile,
                      const std::string &objectFile) {
    std::string command = "gcc -c " + assemblyFile + " -o " + objectFile;
    runCommand(command);
}

// Link the object files to an executable file
void linkToExecutable(const std::vector<std::string> &objectFiles,
                      const std::string &executableFile) {
    std::string command = "gcc";
    for (const auto &objectFile : objectFiles) {
        command += " " + objectFile;
    }
    command += " -o " + executableFile;
    runCommand(command);
}
