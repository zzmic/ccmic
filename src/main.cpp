#include "assembly.h"
#include "assembly_generator.cpp"
#include "compiler.h"
#include "fixup_pass.h"
#include "ir.h"
#include "ir_generator.cpp"
#include "lexer.h"
#include "parser.h"
#include "print_visitor.h"
#include "pseudo_to_stack_pass.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

std::vector<Token> lexer_executor(const std::string &sourceFile);
std::shared_ptr<AST::Program> parser_executor(const std::vector<Token> &tokens);
std::shared_ptr<IR::Program>
ir_generator_executor(std::shared_ptr<AST::Program> astProgram);
std::shared_ptr<Assembly::Program>
codegen_executor(std::shared_ptr<IR::Program> irProgram);
void printIRProgram(std::shared_ptr<IR::Program> irProgram);
void printAssemblyProgram(std::shared_ptr<Assembly::Program> assemblyProgram);
void code_emission_executor(std::shared_ptr<Assembly::Program> assemblyProgram,
                            const std::string &assemblyFile);

int main(int argc, char *argv[]) {
    // Check if the user provided the input file (and suggest the proper usage
    // if not) and extract the source file name and the flag.
    std::string flag;
    std::string sourceFile;
    if (argc < 2 || argc > 3) {
        std::cerr
            << "Usage: " << argv[0]
            << " [--lex] [--parse] [--tacky] [--codegen] [-S] <sourceFile>\n";
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        flag = "";
        sourceFile = argv[1];
    }
    else {
        flag = argv[1];
        sourceFile = argv[2];
        if (flag != "--lex" && flag != "--parse" && flag != "--tacky" &&
            flag != "--codegen" && flag != "-S") {
            std::cerr << "Usage: " << argv[0]
                      << " [--lex] [--parse] [--tacky] [--codegen] [-S] "
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
    bool tillTacky = false;
    bool tillCodegen = false;
    bool tillEmitAssembly = false;

    /*
     * None of the first four options should produce any output files, and all
     * should terminate with an exit code of 0 if successful.
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
    // Direct the compiler to run the lexer, parser, and IR generator, but stop
    // before assembly generation.
    else if (flag == "--tacky") {
        tillTacky = true;
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
    std::vector<Token> tokens = lexer_executor(sourceFile);

    if (tillLex) {
        std::cout << "Lexical tokenization completed successfully.\n";
        return EXIT_SUCCESS;
    }

    // Parse the tokens, generate the AST, visit the AST, and print the AST, and
    // return the AST program.
    std::shared_ptr<AST::Program> astProgram = parser_executor(tokens);

    if (tillParse) {
        std::cout << "Parsing completed successfully.\n";
        return EXIT_SUCCESS;
    }

    // Generate the IR from the AST program and return the IR program.
    std::shared_ptr<IR::Program> irProgram = ir_generator_executor(astProgram);

    // Print the IR program onto the stdout.
    printIRProgram(irProgram);

    if (tillTacky) {
        std::cout << "IR generation completed successfully.\n";
        return EXIT_SUCCESS;
    }

    // Convert the AST program to an IR program and generate the assembly.
    std::shared_ptr<Assembly::Program> assemblyProgram =
        codegen_executor(irProgram);

    // Print out the (assembly) instructions that would be emitted from the
    // assembly program.
    printAssemblyProgram(assemblyProgram);

    if (tillCodegen) {
        std::cout << "Code generation completed successfully.\n";
        return EXIT_SUCCESS;
    }

    // Emit the generated assembly code to the assembly file.
    code_emission_executor(assemblyProgram, assemblyFile);

    if (tillEmitAssembly) {
        std::cout << "Code emission completed successfully.\n";
        return EXIT_SUCCESS;
    }

    // Assemble the assembly file and link it to produce an executable
    // (`outputFile`).
    assembleAndLink(assemblyFile, outputFile);
    // Delete the assebmly file after assembling and linking it.
    std::filesystem::remove(assemblyFile);

    std::cout << "Compilation completed successfully. Output file: "
              << outputFile << "\n";
    return EXIT_SUCCESS;
}

std::vector<Token> lexer_executor(const std::string &sourceFile) {
    // Instantiate an input file stream to read the source file.
    std::ifstream sourceFileInputStream(sourceFile);
    if (sourceFileInputStream.fail()) {
        std::stringstream msg;
        msg << "Unable to open source file: " << sourceFile;
        throw std::runtime_error(msg.str());
    }

    // Read the entire source file into a string.
    // The first iterator is an iterator that reads characters from the input
    // stream.
    // The second iterator is the (default-constructed) end-of-stream iterator.
    // The string range-constructor reads characters from the input stream until
    // the end-of-stream iterator is reached.
    std::string input((std::istreambuf_iterator<char>(sourceFileInputStream)),
                      std::istreambuf_iterator<char>());
    sourceFileInputStream.close();

    std::vector<Token> tokens = lexer(input);
    printTokens(tokens);

    return tokens;
}

// Function to parse the tokens, generate the AST, visit the AST, and print the
// AST.
std::shared_ptr<AST::Program>
parser_executor(const std::vector<Token> &tokens) {
    // Instantiate a parser object with the tokens.
    AST::Parser parser(tokens);

    try {
        // Parse the tokens to generate the AST.
        std::shared_ptr<AST::Program> program = parser.parse();

        // Instantiate a print visitor object.
        AST::PrintVisitor printVisitor;

        // Visit the program to print the AST.
        std::cout << "\n";
        program->accept(printVisitor);

        // Reurn the AST program.
        return program;
    } catch (const std::runtime_error &e) {
        // Throw a runtime error if there is an error during parsing.
        std::stringstream msg;
        msg << "Parsing error: " << e.what();
        throw std::runtime_error(msg.str());
    }

    return EXIT_SUCCESS;
}

// Function to generate the IR from the AST program.
std::shared_ptr<IR::Program>
ir_generator_executor(std::shared_ptr<AST::Program> astProgram) {
    std::cout << "\n";
    // Instantiate an IR generator object and generate the IR.
    std::shared_ptr<IRGenerator> irGenerator;
    return irGenerator->generate(astProgram);
}

// Function to generate (but not yet emit) the assembly program from the AST
// program.
std::shared_ptr<Assembly::Program>
codegen_executor(std::shared_ptr<IR::Program> irProgram) {
    // Instantiate an assembly generator object and generate the assembly.
    std::shared_ptr<Assembly::AssemblyGenerator> assemblyGenerator;
    std::shared_ptr<Assembly::Program> assemblyProgram =
        assemblyGenerator->generate(irProgram);

    // // Print the assembly program before any the following two passes.
    // std::cout << "Before PseudoToStackPass and FixupPass:\n";
    // printAssemblyProgram(assemblyProgram);

    // Instantiate a pseudo-to-stack pass object and return the offset.
    Assembly::PseudoToStackPass pseudoToStackPass;
    int stackSize = pseudoToStackPass.returnOffset(
        assemblyProgram->getFunctionDefinition());

    // Instantiate a fixup pass object and fixup the assembly program.
    Assembly::FixupPass fixupPass;
    fixupPass.fixup(assemblyProgram->getFunctionDefinition(), stackSize);

    // Finally, return the assembly program.
    return assemblyProgram;
}

// Function to print the IR program onto the stdout.
void printIRProgram(std::shared_ptr<IR::Program> irProgram) {
    for (auto function : *irProgram->getFunctionDefinition()) {
        std::cout << function->getFunctionIdentifier() << ":\n";
        for (auto instruction : *function->getFunctionBody()) {
            if (auto returnInstruction =
                    std::dynamic_pointer_cast<IR::ReturnInstruction>(
                        instruction)) {
                if (auto constantValue =
                        std::dynamic_pointer_cast<IR::ConstantValue>(
                            returnInstruction->getReturnValue())) {
                    std::cout << "    return " << constantValue->getValue()
                              << "\n";
                }
                else if (auto variableValue =
                             std::dynamic_pointer_cast<IR::VariableValue>(
                                 returnInstruction->getReturnValue())) {
                    std::cout << "    return " << variableValue->getIdentifier()
                              << "\n";
                }
            }
            else if (auto unaryInstruction =
                         std::dynamic_pointer_cast<IR::UnaryInstruction>(
                             instruction)) {
                if (auto complementOperator =
                        std::dynamic_pointer_cast<IR::ComplementOperator>(
                            unaryInstruction->getUnaryOperator())) {
                    std::cout << "    ~";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                unaryInstruction->getDst())) {
                        std::cout << variableValue->getIdentifier();
                        std::cout << " = ";
                        if (auto variableValue =
                                std::dynamic_pointer_cast<IR::VariableValue>(
                                    unaryInstruction->getSrc())) {
                            std::cout << variableValue->getIdentifier();
                            std::cout << "\n";
                        }
                        else if (auto constantValue = std::dynamic_pointer_cast<
                                     IR::ConstantValue>(
                                     unaryInstruction->getSrc())) {
                            std::cout << constantValue->getValue();
                            std::cout << "\n";
                        }
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     unaryInstruction->getDst())) {
                        std::cout << constantValue->getValue();
                        std::cout << "\n";
                    }
                }
                else if (auto negateOperator =
                             std::dynamic_pointer_cast<IR::NegateOperator>(
                                 unaryInstruction->getUnaryOperator())) {
                    std::cout << "    -";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                unaryInstruction->getDst())) {
                        std::cout << variableValue->getIdentifier();
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     unaryInstruction->getDst())) {
                        std::cout << constantValue->getValue();
                    }
                    std::cout << " = ";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                unaryInstruction->getSrc())) {
                        std::cout << variableValue->getIdentifier();
                        std::cout << "\n";
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     unaryInstruction->getSrc())) {
                        std::cout << constantValue->getValue();
                        std::cout << "\n";
                    }
                }
            }
        }
    }
}

// Function to print the assembly code onto the stdout.
void printAssemblyProgram(std::shared_ptr<Assembly::Program> assemblyProgram) {
    for (auto instruction :
         *assemblyProgram->getFunctionDefinition()->at(0)->getFunctionBody()) {
        if (auto movInstruction =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(
                    instruction)) {
            if (auto src = std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                    movInstruction->getSrc())) {
                if (auto dst =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            movInstruction->getDst())) {
                    std::cout << "movl %" << src->getRegister() << ", %"
                              << dst->getRegister() << "\n";
                }
            }
            else if (auto src =
                         std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                             movInstruction->getSrc())) {
                if (auto dst =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            movInstruction->getDst())) {
                    std::cout << "movl $" << src->getImmediate() << ", %"
                              << dst->getRegister() << "\n";
                }
            }
        }
        else if (auto retInstruction =
                     std::dynamic_pointer_cast<Assembly::RetInstruction>(
                         instruction)) {
            std::cout << "ret\n";
        }
    }
}

// Function to emit the generated assembly code to the assembly file.
void code_emission_executor(std::shared_ptr<Assembly::Program> assemblyProgram,
                            const std::string &assemblyFile) {
    std::ofstream assemblyFileStream(assemblyFile);
    if (!assemblyFileStream.is_open()) {
        std::stringstream msg;
        msg << "Error: Unable to open output file " << assemblyFile << "\n";
        throw std::runtime_error(msg.str());
    }

    auto function = assemblyProgram->getFunctionDefinition();

    std::string functionName = function->at(0)->getFunctionIdentifier();
#ifdef __APPLE__
    functionName = "_" + functionName;
#endif
    assemblyFileStream << "    .globl " << functionName << "\n";
    assemblyFileStream << functionName << ":\n";

    for (auto instruction : *function->at(0)->getFunctionBody()) {
        assemblyFileStream << "    ";
        if (auto movInstruction =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(
                    instruction)) {
            if (auto src = std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                    movInstruction->getSrc())) {
                if (auto dst =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            movInstruction->getDst())) {
                    assemblyFileStream << "movl %" << src->getRegister()
                                       << ", %" << dst->getRegister() << "\n";
                }
            }
            else if (auto src =
                         std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                             movInstruction->getSrc())) {
                if (auto dst =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            movInstruction->getDst())) {
                    assemblyFileStream << "movl $" << src->getImmediate()
                                       << ", %" << dst->getRegister() << "\n";
                }
            }
        }
        else if (auto retInstruction =
                     std::dynamic_pointer_cast<Assembly::RetInstruction>(
                         instruction)) {
            assemblyFileStream << "ret\n";
        }
    }

// If the underlying OS is Linux, add the following to enable an important
// security hardening measure: it indicates that the code does not require an
// executable stack.
#ifdef __linux__
    assemblyFileStream << "\n.section .note.GNU-stack,\"\",@progbits\n";
#endif

    assemblyFileStream.close();
}
