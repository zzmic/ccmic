#include "compilerDriver.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/_types/_pid_t.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
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

namespace {
/**
 * The compiler emits x86-64 assembly, so the host toolchain must be explicitly
 * configured to target x86-64. Otherwise, the driver would, for instance, pass
 * x86-64 assembly to an arm64 assembler on Apple Silicon hosts, which would
 * reject `%rbp`, `$16`, `je`, etc. The target has to be selected per
 * invocation, via `-arch` (or `-target`): no property of the driver process
 * itself, such as running it under `arch -x86_64`, has any bearing on the
 * architecture that `gcc` targets.
 * That is, `arch -x86_64` is not a substitute for the flag, for two independent
 * reasons: (1) `arch` sets a binary preference on the one process it spawns,
 * which does not propagate to the grandchildren that `runCommand` forks
 * (`arch -x86_64 zsh -c 'gcc -v'` still reports `Target: arm64-apple-darwin`);
 * (2) even applied to `gcc` directly it changes nothing, since `/usr/bin/gcc`
 * is a shim that executes an arm64-only Clang.
 */
constexpr std::array<const char *, 3> driverCommand = {
    "gcc",
    "-arch",
    "x86_64",
};

/**
 * Build an argument vector consisting of the driver command followed by `args`.
 */
std::vector<std::string> withDriverCommand(std::vector<std::string> args) {
    // `driverCommand` holds `const char *`s rather than `std::string_view`s
    // because the conversion from `std::string_view` to `std::string` is
    // explicit, which would render the range insert below ill-formed.
    args.insert(args.begin(), driverCommand.begin(), driverCommand.end());
    return args;
}
} // namespace

void preprocess(std::string_view inputFileName,
                std::string_view preprocessedFileName) {
    runCommand(withDriverCommand({
        "-E",
        "-P",
        std::string{inputFileName},
        "-o",
        std::string{preprocessedFileName},
    }));
}

void compileToAssembly(std::string_view preprocessedFileName,
                       std::string_view assemblyFileName) {
    runCommand(withDriverCommand({
        "-S",
        std::string{preprocessedFileName},
        "-o",
        std::string{assemblyFileName},
    }));
}

void assembleToObject(std::string_view assemblyFileName,
                      std::string_view objectFileName) {
    runCommand(withDriverCommand({
        "-c",
        std::string{assemblyFileName},
        "-o",
        std::string{objectFileName},
    }));
}

void linkToExecutable(const std::vector<std::string> &objectFileNames,
                      std::string_view executableFileName) {
    std::vector<std::string> args = objectFileNames;
    args.insert(args.end(), {"-o", std::string{executableFileName}, "-lc"});
    runCommand(withDriverCommand(std::move(args)));
}
