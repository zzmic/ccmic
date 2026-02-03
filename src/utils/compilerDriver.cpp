#include "compilerDriver.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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

void preprocess(std::string_view inputFileName,
                std::string_view preprocessedFileName) {
    auto command = std::string{"gcc -E -P "} + std::string{inputFileName} +
                   " -o " + std::string{preprocessedFileName};
    runCommand(command);
}

void compileToAssembly(std::string_view preprocessedFileName,
                       std::string_view assemblyFileName) {
    auto command = std::string{"gcc -S "} + std::string{preprocessedFileName} +
                   " -o " + std::string{assemblyFileName};
    runCommand(command);
}

void assembleToObject(std::string_view assemblyFileName,
                      std::string_view objectFileName) {
    auto command = std::string{"gcc -c "} + std::string{assemblyFileName} +
                   " -o " + std::string{objectFileName};
    runCommand(command);
}

void linkToExecutable(const std::vector<std::string> &objectFileNames,
                      std::string_view executableFileName) {
    auto command = std::string{"gcc"};
    for (const auto &objectFile : objectFileNames) {
        command += " " + objectFile;
    }
    command += " -o " + std::string{executableFileName};
    // Link against the C standard library (libc).
    command += " -lc";
    runCommand(command);
}
