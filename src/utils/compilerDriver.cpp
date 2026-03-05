#include "compilerDriver.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/_types/_pid_t.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void runCommand(const std::vector<std::string> &args) {
    if (args.empty()) {
        throw std::runtime_error("Empty command");
    }

    std::cout << "Executing command:";
    for (const auto &arg : args) {
        std::cout << ' ' << arg;
    }
    std::cout << '\n';

    // `execvp` expects a `char* const[]` (non-const pointers),
    // so we may not be able to use the original `const std::string&` args
    // directly. Thus, we create a mutable copy of the arguments and convert
    // them to `char*`s before calling `execvp`.
    std::vector<std::string> mutableArgsCopy = args;
    std::vector<char *> argv;
    argv.reserve(mutableArgsCopy.size());
    for (auto &arg : mutableArgsCopy) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);

    // Fork the process and execute the command in the child process.
    const pid_t pid = fork();
    // If `fork` returns a negative value, it means the fork failed. If it
    // returns zero, it means we are in the child process. If it returns a
    // positive value, it means we are in the parent process and the return
    // value is the PID of the child process.
    if (pid < 0) {
        throw std::runtime_error("fork() failed");
    }
    if (pid == 0) {
        // In the child process, execute the command.
        // If `execvp` returns, it means there was an error.
        execvp(argv[0], argv.data());
        std::perror("execvp");
        std::exit(EXIT_FAILURE);
    }

    int status = 0;
    // In the parent process, wait for the child process to finish and check its
    // exit status.
    if (waitpid(pid, &status, 0) == -1) {
        throw std::runtime_error("waitpid() failed");
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        // The command failed, either by exiting with a non-zero status or by
        // being terminated by a signal.
        throw std::runtime_error("Command failed!");
    }
}

void preprocess(std::string_view inputFileName,
                std::string_view preprocessedFileName) {
    runCommand({"gcc", "-E", "-P", std::string{inputFileName}, "-o",
                std::string{preprocessedFileName}});
}

void compileToAssembly(std::string_view preprocessedFileName,
                       std::string_view assemblyFileName) {
    runCommand({"gcc", "-S", std::string{preprocessedFileName}, "-o",
                std::string{assemblyFileName}});
}

void assembleToObject(std::string_view assemblyFileName,
                      std::string_view objectFileName) {
    runCommand({"gcc", "-c", std::string{assemblyFileName}, "-o",
                std::string{objectFileName}});
}

void linkToExecutable(const std::vector<std::string> &objectFileNames,
                      std::string_view executableFileName) {
    std::vector<std::string> args = {"gcc"};
    for (const auto &obj : objectFileNames) {
        args.push_back(obj);
    }
    args.insert(args.end(), {"-o", std::string{executableFileName}, "-lc"});
    runCommand(args);
}
