#include "compilerDriver.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

void runCommand(std::string_view command) {
    std::cout << "Executing command: " << command << "\n";
    // Execute `command` as a shell command and get the result.
    auto result = system(std::string{command}.c_str());

    // If the command failed, the result is a non-zero exit code (and the system
    // shouldn't write (to) any assembly or executable files).
    if (result != 0) {
        std::stringstream msg;
        msg << "Command " << command << " failed!";
        throw std::runtime_error(msg.str());
    }
}

void preprocess(std::string_view inputFile, std::string_view preprocessedFile) {
    auto command = std::string{"gcc -E -P "} + std::string{inputFile} + " -o " +
                   std::string{preprocessedFile};
    runCommand(command);
}

void compileToAssembly(std::string_view preprocessedFile,
                       std::string_view assemblyFile) {
    auto command = std::string{"gcc -S "} + std::string{preprocessedFile} +
                   " -o " + std::string{assemblyFile};
    runCommand(command);
}

void assembleToObject(std::string_view assemblyFile,
                      std::string_view objectFile) {
    auto command = std::string{"gcc -c "} + std::string{assemblyFile} + " -o " +
                   std::string{objectFile};
    runCommand(command);
}

void linkToExecutable(const std::vector<std::string> &objectFiles,
                      std::string_view executableFile) {
    auto command = std::string{"gcc"};
    for (const auto &objectFile : objectFiles) {
        command += " " + objectFile;
    }
    command += " -o " + std::string{executableFile};
    // Link against the C standard library (libc).
    command += " -lc";
    runCommand(command);
}
