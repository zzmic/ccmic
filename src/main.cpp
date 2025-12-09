#include "utils/pipelineStagesExecutors.h"
#include "utils/prettyPrinters.h"

int main(int argc, char *argv[]) {
    try {
        // Check if the user provided the input file (and suggest the proper
        // usage if not) and extract the source file (name) and the flag(s).
        std::vector<std::string> flags;
        std::string sourceFile;
        if (argc < 2) {
            std::cerr
                << "Usage: " << argv[0]
                << " [--lex] [--parse] [--validate] [--tacky] [--codegen] "
                   "[-S] [-s] [-c] [-o] [--fold-constants] "
                   "[--propagate-copies] "
                   "[--eliminate-unreachable-code] [--eliminate-dead-stores] "
                   "[--optimize] <sourceFile>\n";
            std::cerr << "Given argc: " << argc << "\n";
            return EXIT_FAILURE;
        }
        // Parse the command line arguments and extract the flag(s) and the
        // source file (name).
        for (int i = 1; i < argc - 1; ++i) {
            flags.emplace_back(argv[i]);
        }
        sourceFile = argv[argc - 1];

        // Initialize flags to control the intermediate stages of the
        // compilation.
        bool tillLex = false;
        bool tillParse = false;
        bool tillValidate = false;
        bool tillIR = false;
        bool tillCodegen = false;
        bool tillEmitAssembly = false;
        bool tillObject = false;
        bool foldConstantsPass = false;
        bool propagateCopiesPass = false;
        bool eliminateUnreachableCodePass = false;
        bool eliminateDeadStoresPass = false;
        for (const auto &flag : flags) {
            // Direct the compiler to run the lexer, but stop before the parser.
            if (flag == "--lex") {
                tillLex = true;
            }
            // Direct the compiler to run the lexer and parser, but stop before
            // assembly generation.
            else if (flag == "--parse") {
                tillParse = true;
            }
            // Direct the compiler to run the lexer, parser, and validator, but
            // stop before IR generation.
            else if (flag == "--validate") {
                tillValidate = true;
            }
            // Direct the compiler to run the lexer, parser, and IR generator,
            // but stop before assembly generation.
            else if (flag == "--tacky") {
                tillIR = true;
            }
            // Direct the compiler to perform lexing, parsing, IR generation,
            // and assembly generation, but stop before assembly emission.
            else if (flag == "--codegen") {
                tillCodegen = true;
            }
            // Direct the compiler to emit the assembly file, but not to
            // assemble and link it.
            else if (flag == "-S" || flag == "-s") {
                tillEmitAssembly = true;
            }
            // Direct the compiler to compile the source file into an object
            // file without linking it into an executable.
            else if (flag == "-c") {
                tillObject = true;
            }
            // Direct the compiler to fold constants.
            else if (flag == "--fold-constants") {
                foldConstantsPass = true;
            }
            // Direct the compiler to propagate copies.
            else if (flag == "--propagate-copies") {
                propagateCopiesPass = true;
            }
            // Direct the compiler to eliminate unreachable code.
            else if (flag == "--eliminate-unreachable-code") {
                eliminateUnreachableCodePass = true;
            }
            // Direct the compiler to eliminate dead stores.
            else if (flag == "--eliminate-dead-stores") {
                eliminateDeadStoresPass = true;
            }
            // Direct the compiler to perform all the optimization passes.
            else if (flag == "--optimize") {
                foldConstantsPass = true;
                propagateCopiesPass = true;
                eliminateUnreachableCodePass = true;
                eliminateDeadStoresPass = true;
            }
            else {
                std::cerr << "Unknown/invalid flag: " << flag << "\n";
                return EXIT_FAILURE;
            }
        }

        // Extract the program (base/actual) name of the source file (without
        // the extension).
        size_t dotIndex = sourceFile.rfind('.');
        if (dotIndex == static_cast<size_t>(-1)) {
            std::stringstream msg;
            msg << "Source file must have an extension.";
            throw std::invalid_argument(msg.str());
        }
        if (dotIndex == 0 ||
            dotIndex == static_cast<size_t>(sourceFile.size() - 1)) {
            std::stringstream msg;
            msg << "Source file must have a name before and after the "
                   "extension.";
            throw std::invalid_argument(msg.str());
        }
        if (sourceFile[dotIndex + 1] != 'c') {
            std::stringstream msg;
            msg << "Source file must have a '.c' extension.";
            throw std::invalid_argument(msg.str());
        }
        std::string programName = sourceFile.substr(0, sourceFile.rfind('.'));
        // Construct the preprocessed file name by appending the ".i" extension.
        std::string preprocessedFileName = programName + ".i";
        // Construct the assembly file name by appending the ".s" extension.
        std::string assemblyFileName = programName + ".s";
        // Construct the object file name by appending the ".o" extension.
        std::string objectFileName = programName + ".o";
        // Construct a vector to store the object files.
        // Note: For now, there is only one object file.
        std::vector<std::string> objectFileNames;
        // Construct the executable file name.
        std::string executableFileName = programName;

        // Preprocess the source file and write the result to the preprocessed.
        preprocess(sourceFile, preprocessedFileName);

        // Tokenize the input, print the tokens, and return the tokens.
        auto tokens =
            PipelineStagesExecutors::lexerExecutor(preprocessedFileName);

        // Delete the preprocessed file after compiling it to assembly.
        std::filesystem::remove(preprocessedFileName);

        if (tillLex) {
            std::cout << "Lexical tokenization completed.\n";
            return EXIT_SUCCESS;
        }

        // Parse the tokens, generate the AST, visit the AST, and print the AST,
        // and return the AST program.
        auto astProgram = PipelineStagesExecutors::parserExecutor(tokens);

        if (tillParse) {
            std::cout << "Parsing completed.\n";
            return EXIT_SUCCESS;
        }

        // Perform semantic analysis on the AST program.
        auto variableResolutionCounter =
            PipelineStagesExecutors::semanticAnalysisExecutor(astProgram);

        if (tillValidate) {
            std::cout << "Semantic analysis completed.\n";
            return EXIT_SUCCESS;
        }

        // Generate the IR from the AST program and return the IR program.
        auto irProgramAndIRStaticVariables =
            PipelineStagesExecutors::irGeneratorExecutor(
                astProgram, variableResolutionCounter);
        auto irProgram = irProgramAndIRStaticVariables.first;
        auto irStaticVariables = irProgramAndIRStaticVariables.second;

        if (foldConstantsPass || propagateCopiesPass ||
            eliminateUnreachableCodePass || eliminateDeadStoresPass) {
            // Print the IR program to stdout.
            std::cout << "<<< Before optimization passes: >>>\n";
            PrettyPrinters::printIRProgram(irProgram, irStaticVariables);

            // Perform the optimization passes on the IR program (if any of the
            // flags is set to true).
            PipelineStagesExecutors::irOptimizationExecutor(
                irProgram, foldConstantsPass, propagateCopiesPass,
                eliminateUnreachableCodePass, eliminateDeadStoresPass);

            // Print the optimized IR program to stdout (after the
            // optimization passes).
            std::cout << "<<< After optimization passes: >>>\n";
            PrettyPrinters::printIRProgram(irProgram, irStaticVariables);
        }
        else {
            // Print the IR program to stdout.
            PrettyPrinters::printIRProgram(irProgram, irStaticVariables);
        }

        if (tillIR) {
            std::cout
                << "IR generation (and potential executions of optimization "
                   "passes) completed.\n";
            return EXIT_SUCCESS;
        }

        // Generate the assembly program from the IR program and the IR static
        // variables.
        std::shared_ptr<Assembly::Program> assemblyProgram =
            PipelineStagesExecutors::codegenExecutor(irProgram,
                                                     irStaticVariables);

        // Print out the (assembly) instructions that would be emitted from the
        // assembly program.
        PrettyPrinters::printAssemblyProgram(assemblyProgram);

        if (tillCodegen) {
            std::cout << "Code generation completed.\n";
            return EXIT_SUCCESS;
        }

        // Emit the generated assembly code to the assembly file.
        PipelineStagesExecutors::codeEmissionExecutor(assemblyProgram,
                                                      assemblyFileName);

        if (tillEmitAssembly) {
            std::cout << "Code emission completed.\n";
            return EXIT_SUCCESS;
        }

        // Assemble the assembly file to an object file and add it to the object
        // file vector.
        assembleToObject(assemblyFileName, objectFileName);
        objectFileNames.emplace_back(objectFileName);

        // Delete the assebmly file after assembling and linking it.
        std::filesystem::remove(assemblyFileName);

        if (tillObject) {
            std::cout << "Compilation completed. Object file: "
                      << objectFileName << "\n";
            return EXIT_SUCCESS;
        }

        // Link the object files to an executable file.
        linkToExecutable(objectFileNames, executableFileName);

        // Remove the object files after linking them to the executable.
        for (const auto &objFileName : objectFileNames) {
            std::filesystem::remove(objFileName);
        }

        std::cout << "Compilation completed. Executable file: "
                  << executableFileName << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
}
