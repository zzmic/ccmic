#include "pipelineStagesExecutors.h"
#include "prettyPrinters.h"

int main(int argc, char *argv[]) {
    // Check if the user provided the input file (and suggest the proper usage
    // if not) and extract the source file name and the flag.
    std::string flag;
    std::string sourceFile;
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0]
                  << " [--lex] [--parse] [--validate] [--tacky] [--codegen] "
                     "[-S] [-c] [-o] <sourceFile>\n";
        std::cerr << "Given argc: " << argc << "\n";
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        flag = "";
        sourceFile = argv[1];
    }
    else {
        flag = argv[1];
        sourceFile = argv[2];
        if (!(flag == "--lex" || flag == "--parse" || flag == "--validate" ||
              flag == "--tacky" || flag == "--codegen" || flag == "-S" ||
              flag == "-c" || flag == "-o")) {
            std::cerr
                << "Usage: " << argv[0]
                << " [--lex] [--parse] [--validate] [--tacky] [--codegen] [-S] "
                   "[-c] [-o] <sourceFile>\n";
            std::cerr << "Given flag: " << flag << "\n";
            return EXIT_FAILURE;
        }
    }

    // Extract the program (base/actual) name of the source file (without the
    // extension).
    std::size_t dotIndex = sourceFile.rfind('.');
    if (dotIndex == static_cast<std::size_t>(-1)) {
        std::stringstream msg;
        msg << "Source file must have an extension.";
        throw std::runtime_error(msg.str());
    }
    if (dotIndex == 0 ||
        dotIndex == static_cast<std::size_t>(sourceFile.size() - 1)) {
        std::stringstream msg;
        msg << "Source file must have a name before and after the extension.";
        throw std::runtime_error(msg.str());
    }
    if (sourceFile[dotIndex + 1] != 'c') {
        std::stringstream msg;
        msg << "Source file must have a '.c' extension.";
        throw std::runtime_error(msg.str());
    }
    std::string programName = sourceFile.substr(0, sourceFile.rfind('.'));

    // Construct the preprocessed file name by appending the ".i" extension.
    std::string preprocessedFile = programName + ".i";
    // Construct the assembly file name by appending the ".s" extension.
    std::string assemblyFile = programName + ".s";
    // Construct the object file name by appending the ".o" extension.
    std::string objectFile = programName + ".o";
    // Construct a vector to store the object files.
    // Note: For now, there is only one object file.
    std::vector<std::string> objectFiles;
    // Construct the executable file name.
    std::string executableFile = programName;

    // Initialize flags to control the intermediate stages of the compilation.
    bool tillLex = false;
    bool tillParse = false;
    bool tillValidate = false;
    bool tillIR = false;
    bool tillCodegen = false;
    bool tillEmitAssembly = false;
    bool tillObject = false;
    /*
     * None of the first five options should produce any output files, and all
     * should terminate with an exit code of 0 if no runtime errors occur.
     */
    // Direct the compiler to run the lexer, but stop before the parser.
    if (flag == "--lex") {
        tillLex = true;
    }
    // Direct the compiler to run the lexer and parser, but stop before
    // assembly generation.
    else if (flag == "--parse") {
        tillParse = true;
    }
    // Direct the compiler to run the lexer, parser, and validator, but stop
    // before IR generation.
    else if (flag == "--validate") {
        tillValidate = true;
    }
    // Direct the compiler to run the lexer, parser, and IR generator, but stop
    // before assembly generation.
    else if (flag == "--tacky") {
        tillIR = true;
    }
    // Direct the compiler to perform lexing, parsing, IR generation, and
    // assembly generation, but stop before assembly emission.
    else if (flag == "--codegen")
        tillCodegen = true;
    // Direct the compiler to emit the assembly file, but not to assemble and
    // link it.
    else if (flag == "-S")
        tillEmitAssembly = true;
    // Direct the compiler to compile the source file into an object file
    // without linking it into an executable.
    else if (flag == "-c")
        tillObject = true;

    // Preprocess the source file and write the result to the preprocessed.
    preprocess(sourceFile, preprocessedFile);

    // Tokenize the input, print the tokens, and return the tokens.
    auto tokens = PipelineStagesExecutors::lexerExecutor(preprocessedFile);

    // Delete the preprocessed file after compiling it to assembly.
    std::filesystem::remove(preprocessedFile);

    if (tillLex) {
        std::cout << "Lexical tokenization completed.\n";
        return EXIT_SUCCESS;
    }

    // Parse the tokens, generate the AST, visit the AST, and print the AST, and
    // return the AST program.
    auto astProgram = PipelineStagesExecutors::parserExecutor(tokens);

    if (tillParse) {
        std::cout << "Parsing completed.\n";
        return EXIT_SUCCESS;
    }

    // Perform semantic analysis on the AST program.
    auto result = PipelineStagesExecutors::semanticAnalysisExecutor(astProgram);
    // Extract the variable resolution counter and the symbol table from the
    // result since they are needed for the later stages.
    auto variableResolutionCounter = result.first;
    auto symbols = result.second;

    if (tillValidate) {
        std::cout << "Semantic analysis completed.\n";
        return EXIT_SUCCESS;
    }

    // Generate the IR from the AST program and return the IR program.
    auto irProgramAndIRStaticVariables =
        PipelineStagesExecutors::irGeneratorExecutor(
            astProgram, variableResolutionCounter, symbols);
    auto irProgram = irProgramAndIRStaticVariables.first;
    auto irStaticVariables = irProgramAndIRStaticVariables.second;

    // Print the IR program onto the stdout.
    PrettyPrinters::printIRProgram(irProgram, irStaticVariables);

    if (tillIR) {
        std::cout << "IR generation completed.\n";
        return EXIT_SUCCESS;
    }

    // Generate the assembly program from the IR program and the IR static
    // variables.
    std::shared_ptr<Assembly::Program> assemblyProgram =
        PipelineStagesExecutors::codegenExecutor(irProgram, irStaticVariables,
                                                 symbols);

    // Print out the (assembly) instructions that would be emitted from the
    // assembly program.
    PrettyPrinters::printAssemblyProgram(assemblyProgram);

    if (tillCodegen) {
        std::cout << "Code generation completed.\n";
        return EXIT_SUCCESS;
    }

    // Emit the generated assembly code to the assembly file.
    PipelineStagesExecutors::codeEmissionExecutor(assemblyProgram,
                                                  assemblyFile);

    if (tillEmitAssembly) {
        std::cout << "Code emission completed.\n";
        return EXIT_SUCCESS;
    }

    // Assemble the assembly file to an object file and add it to the object
    // file vector.
    assembleToObject(assemblyFile, objectFile);
    objectFiles.emplace_back(objectFile);

    // Delete the assebmly file after assembling and linking it.
    std::filesystem::remove(assemblyFile);

    if (tillObject) {
        std::cout << "Compilation completed. Object file: " << objectFile
                  << "\n";
        return EXIT_SUCCESS;
    }

    // Link the object files to an executable file.
    linkToExecutable(objectFiles, executableFile);

    // Remove the object file after linking it to the executable.
    std::filesystem::remove(objectFile);

    std::cout << "Compilation completed. Executable file: " << executableFile
              << "\n";
    return EXIT_SUCCESS;
}
