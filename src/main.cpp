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
                     "[-S] <sourceFile>\n";
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
              flag == "--tacky" || flag == "--codegen" || flag == "-S")) {
            std::cerr
                << "Usage: " << argv[0]
                << " [--lex] [--parse] [--validate] [--tacky] [--codegen] [-S] "
                   "<sourceFile>\n";
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
    // Construct the output file name by using the program name.
    std::string outputFile = programName;

    // // Preprocess the source file and write the result to the preprocessed.
    // file preprocess(sourceFile, preprocessedFile);
    // // Compile the preprocessed file to assembly and write the result to the
    // // assembly file.
    // compileToAssembly(preprocessedFile, assemblyFile);
    // // Delete the preprocessed file after compiling it to assembly.
    // std::filesystem::remove(preprocessedFile);

    // Initialize flags to control the intermediate stages of the compilation.
    bool tillLex = false;
    bool tillParse = false;
    bool tillValidate = false;
    bool tillIR = false;
    bool tillCodegen = false;
    bool tillEmitAssembly = false;

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

    // Tokenize the input, print the tokens, and return the tokens.
    std::vector<Token> tokens =
        PipelineStagesExecutors::lexerExecutor(sourceFile);

    if (tillLex) {
        std::cout << "Lexical tokenization completed.\n";
        return EXIT_SUCCESS;
    }

    // Parse the tokens, generate the AST, visit the AST, and print the AST, and
    // return the AST program.
    std::shared_ptr<AST::Program> astProgram =
        PipelineStagesExecutors::parserExecutor(tokens);

    if (tillParse) {
        std::cout << "Parsing completed.\n";
        return EXIT_SUCCESS;
    }

    // Perform semantic analysis on the AST program and return the global
    // counter.
    int variableResolutionCounter =
        PipelineStagesExecutors::semanticAnalysisExecutor(astProgram);

    if (tillValidate) {
        std::cout << "Semantic analysis completed.\n";
        return EXIT_SUCCESS;
    }

    // Generate the IR from the AST program and return the IR program.
    std::shared_ptr<IR::Program> irProgram =
        PipelineStagesExecutors::irGeneratorExecutor(astProgram,
                                                     variableResolutionCounter);

    // Print the IR program onto the stdout.
    PrettyPrinters::printIRProgram(irProgram);

    if (tillIR) {
        std::cout << "IR generation completed.\n";
        return EXIT_SUCCESS;
    }

    // Convert the AST program to an IR program and generate the assembly.
    std::shared_ptr<Assembly::Program> assemblyProgram =
        PipelineStagesExecutors::codegenExecutor(irProgram);

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

    // Assemble the assembly file and link it to produce an executable
    // (`outputFile`).
    assembleAndLink(assemblyFile, outputFile);
    // Delete the assebmly file after assembling and linking it.
    std::filesystem::remove(assemblyFile);

    std::cout << "Compilation completed. Output file: " << outputFile << "\n";
    return EXIT_SUCCESS;
}
