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
    bool tillIR = false;
    bool tillCodegen = false;
    bool tillEmitAssembly = false;

    /*
     * None of the first four options should produce any output files, and all
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
    std::vector<Token> tokens = lexer_executor(sourceFile);

    if (tillLex) {
        std::cout << "Lexical tokenization completed.\n";
        return EXIT_SUCCESS;
    }

    // Parse the tokens, generate the AST, visit the AST, and print the AST, and
    // return the AST program.
    std::shared_ptr<AST::Program> astProgram = parser_executor(tokens);

    if (tillParse) {
        std::cout << "Parsing completed.\n";
        return EXIT_SUCCESS;
    }

    // Generate the IR from the AST program and return the IR program.
    std::shared_ptr<IR::Program> irProgram = ir_generator_executor(astProgram);

    // Print the IR program onto the stdout.
    printIRProgram(irProgram);

    if (tillIR) {
        std::cout << "IR generation completed.\n";
        return EXIT_SUCCESS;
    }

    // Convert the AST program to an IR program and generate the assembly.
    std::shared_ptr<Assembly::Program> assemblyProgram =
        codegen_executor(irProgram);

    // Print out the (assembly) instructions that would be emitted from the
    // assembly program.
    printAssemblyProgram(assemblyProgram);

    if (tillCodegen) {
        std::cout << "Code generation completed.\n";
        return EXIT_SUCCESS;
    }

    // Emit the generated assembly code to the assembly file.
    code_emission_executor(assemblyProgram, assemblyFile);

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

    // Instantiate a pseudo-to-stack pass object and return the offset.
    Assembly::PseudoToStackPass pseudoToStackPass;
    auto functionDefinition = assemblyProgram->getFunctionDefinition();
    int stackSize = pseudoToStackPass.replacePseudoWithStackAndReturnOffset(
        functionDefinition);

    // Instantiate a fixup pass object and fixup the assembly program.
    Assembly::FixupPass fixupPass;
    fixupPass.fixup(functionDefinition, stackSize);
    assemblyProgram->setFunctionDefinition(functionDefinition);

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
                    std::cout << "    ";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                unaryInstruction->getDst())) {
                        std::cout << variableValue->getIdentifier();
                        std::cout << " = ~";
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
                    std::cout << "    ";
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
                    std::cout << " = -";
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
            else if (auto binaryInstruction =
                         std::dynamic_pointer_cast<IR::BinaryInstruction>(
                             instruction)) {
                if (auto variableValue =
                        std::dynamic_pointer_cast<IR::VariableValue>(
                            binaryInstruction->getDst())) {
                    std::cout << "    " << variableValue->getIdentifier();
                    std::cout << " = ";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                binaryInstruction->getLhs())) {
                        std::cout << variableValue->getIdentifier();
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     binaryInstruction->getLhs())) {
                        std::cout << constantValue->getValue();
                    }
                    if (auto binaryOperator =
                            std::dynamic_pointer_cast<IR::AddOperator>(
                                binaryInstruction->getBinaryOperator())) {
                        std::cout << " + ";
                    }
                    else if (auto binaryOperator = std::dynamic_pointer_cast<
                                 IR::SubtractOperator>(
                                 binaryInstruction->getBinaryOperator())) {
                        std::cout << " - ";
                    }
                    else if (auto binaryOperator = std::dynamic_pointer_cast<
                                 IR::MultiplyOperator>(
                                 binaryInstruction->getBinaryOperator())) {
                        std::cout << " * ";
                    }
                    else if (auto binaryOperator =
                                 std::dynamic_pointer_cast<IR::DivideOperator>(
                                     binaryInstruction->getBinaryOperator())) {
                        std::cout << " / ";
                    }
                    else if (auto binaryOperator = std::dynamic_pointer_cast<
                                 IR::RemainderOperator>(
                                 binaryInstruction->getBinaryOperator())) {
                        std::cout << " % ";
                    }
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                binaryInstruction->getRhs())) {
                        std::cout << variableValue->getIdentifier();
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     binaryInstruction->getRhs())) {
                        std::cout << constantValue->getValue();
                    }
                    std::cout << "\n";
                }
                else if (auto constantValue =
                             std::dynamic_pointer_cast<IR::ConstantValue>(
                                 binaryInstruction->getDst())) {
                    std::cout << "    " << constantValue->getValue();
                    std::cout << " = ";
                    if (auto variableValue =
                            std::dynamic_pointer_cast<IR::VariableValue>(
                                binaryInstruction->getLhs())) {
                        std::cout << variableValue->getIdentifier();
                    }
                    else if (auto constantValue =
                                 std::dynamic_pointer_cast<IR::ConstantValue>(
                                     binaryInstruction->getLhs())) {
                        std::cout << constantValue->getValue();
                    }
                }
            }
        }
    }
}

// Function to print the assembly code onto the stdout.
void printAssemblyProgram(std::shared_ptr<Assembly::Program> assemblyProgram) {
    // For now, assume that there is only one function in the program.
    auto function = assemblyProgram->getFunctionDefinition()->at(0);
    auto functionBody =
        assemblyProgram->getFunctionDefinition()->at(0)->getFunctionBody();

    // Print the function prologue (before printing the function body).
    std::cout << "\n"
              << "    .globl " << function->getFunctionIdentifier() << "\n";
    std::cout << function->getFunctionIdentifier() << ":\n";
    std::cout << "    pushq %rbp\n";
    std::cout << "    movq %rsp, %rbp\n";

    // Print the function body.
    for (auto instruction : *functionBody) {
        if (auto movInstruction =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(
                    instruction)) {
            auto src = movInstruction->getSrc();
            auto dst = movInstruction->getDst();
            if (auto srcReg =
                    std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    std::cout << "    movl %" << srcReg->getRegister() << ", %"
                              << dstReg->getRegister() << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    std::cout << "    movl %" << srcReg->getRegister() << ", "
                              << dstStack->getOffset() << "(%rbp)\n";
                }
            }
            else if (auto srcImm =
                         std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                             src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    std::cout << "    movl $" << srcImm->getImmediate() << ", %"
                              << dstReg->getRegister() << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    std::cout << "    movl $" << srcImm->getImmediate() << ", "
                              << dstStack->getOffset() << "(%rbp)\n";
                }
            }
            else if (auto srcStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    std::cout << "    movl " << srcStack->getOffset()
                              << "(%rbp), %" << dstReg->getRegister() << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    std::cout << "    movl " << srcStack->getOffset()
                              << "(%rbp), " << dstStack->getOffset()
                              << "(%rbp)\n";
                }
            }
        }
        else if (auto retInstruction =
                     std::dynamic_pointer_cast<Assembly::RetInstruction>(
                         instruction)) {
            // Print the function epilogue before printing the return
            // instruction.
            std::cout << "    movq %rbp, %rsp\n";
            std::cout << "    popq %rbp\n";
            std::cout << "    ret\n";
        }
        else if (auto allocateStackInstruction = std::dynamic_pointer_cast<
                     Assembly::AllocateStackInstruction>(instruction)) {
            std::cout
                << "    subq $"
                << allocateStackInstruction->getAddressGivenOffsetFromRBP()
                << ", %rsp\n";
        }
        else if (auto unaryInstruction =
                     std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                         instruction)) {
            if (auto negateOperator =
                    std::dynamic_pointer_cast<Assembly::NegateOperator>(
                        unaryInstruction->getUnaryOperator())) {
                std::cout << "    negl";
            }
            else if (auto complementOperator = std::dynamic_pointer_cast<
                         Assembly::ComplementOperator>(
                         unaryInstruction->getUnaryOperator())) {
                std::cout << "    notl";
            }
            if (auto operand = unaryInstruction->getOperand()) {
                if (auto regOperand =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            operand)) {
                    std::cout << " %" << regOperand->getRegister() << "\n";
                }
                else if (auto stackOperand =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 operand)) {
                    std::cout << " " << stackOperand->getOffset() << "(%rbp)\n";
                }
            }
        }
        else if (auto binaryInstruction =
                     std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                         instruction)) {
            if (auto addOperator =
                    std::dynamic_pointer_cast<Assembly::AddOperator>(
                        binaryInstruction->getBinaryOperator())) {
                std::cout << "    addl";
            }
            else if (auto subtractOperator =
                         std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                             binaryInstruction->getBinaryOperator())) {
                std::cout << "    subl";
            }
            else if (auto multiplyOperator =
                         std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                             binaryInstruction->getBinaryOperator())) {
                std::cout << "    imull";
            }
            auto src = binaryInstruction->getOperand1();
            auto dst = binaryInstruction->getOperand2();
            if (auto srcImm =
                    std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                        src)) {
                std::cout << " $" << srcImm->getImmediate() << ",";
            }
            else if (auto srcReg =
                         std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                             src)) {
                std::cout << " %" << srcReg->getRegister() << ",";
            }
            else if (auto srcStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             src)) {
                std::cout << " " << srcStack->getOffset() << "(%rbp),";
            }
            if (auto dstReg =
                    std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
                std::cout << " %" << dstReg->getRegister() << "\n";
            }
            else if (auto dstStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             dst)) {
                std::cout << " " << dstStack->getOffset() << "(%rbp)\n";
            }
        }
        else if (auto idivInstruction =
                     std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                         instruction)) {
            if (auto operand = idivInstruction->getOperand()) {
                if (auto regOperand =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            operand)) {
                    std::cout << "    idivl %" << regOperand->getRegister()
                              << "\n";
                }
                else if (auto stackOperand =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 operand)) {
                    std::cout << "    idivl " << stackOperand->getOffset()
                              << "(%rbp)\n";
                }
            }
        }
        else if (auto cdqInstruction =
                     std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                         instruction)) {
            std::cout << "    cdq\n";
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

    // For now, assume that there is only one function in the program.
    auto function = assemblyProgram->getFunctionDefinition()->at(0);
    std::string functionName = function->getFunctionIdentifier();
    auto functionBody =
        assemblyProgram->getFunctionDefinition()->at(0)->getFunctionBody();
#ifdef __APPLE__
    functionName = "_" + functionName;
#endif

    // Emit the function prologue (before emitting the function body).
    assemblyFileStream << "    .globl " << functionName << "\n";
    assemblyFileStream << functionName << ":\n";
    assemblyFileStream << "    pushq %rbp\n";
    assemblyFileStream << "    movq %rsp, %rbp\n";

    // Emit the function body.
    for (auto instruction : *functionBody) {
        if (auto movInstruction =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(
                    instruction)) {
            auto src = movInstruction->getSrc();
            auto dst = movInstruction->getDst();
            if (auto srcReg =
                    std::dynamic_pointer_cast<Assembly::RegisterOperand>(src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    assemblyFileStream << "    movl %" << srcReg->getRegister()
                                       << ", %" << dstReg->getRegister()
                                       << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    assemblyFileStream << "    movl %" << srcReg->getRegister()
                                       << ", " << dstStack->getOffset()
                                       << "(%rbp)\n";
                }
            }
            else if (auto srcImm =
                         std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                             src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    assemblyFileStream << "    movl $" << srcImm->getImmediate()
                                       << ", %" << dstReg->getRegister()
                                       << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    assemblyFileStream << "    movl $" << srcImm->getImmediate()
                                       << ", " << dstStack->getOffset()
                                       << "(%rbp)\n";
                }
            }
            else if (auto srcStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             src)) {
                if (auto dstReg =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            dst)) {
                    assemblyFileStream << "    movl " << srcStack->getOffset()
                                       << "(%rbp), %" << dstReg->getRegister()
                                       << "\n";
                }
                else if (auto dstStack =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 dst)) {
                    assemblyFileStream << "    movl " << srcStack->getOffset()
                                       << "(%rbp), " << dstStack->getOffset()
                                       << "(%rbp)\n";
                }
            }
        }
        else if (auto retInstruction =
                     std::dynamic_pointer_cast<Assembly::RetInstruction>(
                         instruction)) {
            // Print the function epilogue before printing the return
            // instruction.
            assemblyFileStream << "    movq %rbp, %rsp\n";
            assemblyFileStream << "    popq %rbp\n";
            assemblyFileStream << "    ret\n";
        }
        else if (auto allocateStackInstruction = std::dynamic_pointer_cast<
                     Assembly::AllocateStackInstruction>(instruction)) {
            assemblyFileStream
                << "    subq $"
                << allocateStackInstruction->getAddressGivenOffsetFromRBP()
                << ", %rsp\n";
        }
        else if (auto unaryInstruction =
                     std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                         instruction)) {
            if (auto negateOperator =
                    std::dynamic_pointer_cast<Assembly::NegateOperator>(
                        unaryInstruction->getUnaryOperator())) {
                assemblyFileStream << "    negl";
            }
            else if (auto complementOperator = std::dynamic_pointer_cast<
                         Assembly::ComplementOperator>(
                         unaryInstruction->getUnaryOperator())) {
                assemblyFileStream << "    notl";
            }
            if (auto operand = unaryInstruction->getOperand()) {
                if (auto regOperand =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            operand)) {
                    assemblyFileStream << " %" << regOperand->getRegister()
                                       << "\n";
                }
                else if (auto stackOperand =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 operand)) {
                    assemblyFileStream << " " << stackOperand->getOffset()
                                       << "(%rbp)\n";
                }
            }
        }
        else if (auto binaryInstruction =
                     std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                         instruction)) {
            if (auto addOperator =
                    std::dynamic_pointer_cast<Assembly::AddOperator>(
                        binaryInstruction->getBinaryOperator())) {
                assemblyFileStream << "    addl";
            }
            else if (auto subtractOperator =
                         std::dynamic_pointer_cast<Assembly::SubtractOperator>(
                             binaryInstruction->getBinaryOperator())) {
                assemblyFileStream << "    subl";
            }
            else if (auto multiplyOperator =
                         std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                             binaryInstruction->getBinaryOperator())) {
                assemblyFileStream << "    imull";
            }
            auto src = binaryInstruction->getOperand1();
            auto dst = binaryInstruction->getOperand2();
            if (auto srcImm =
                    std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                        src)) {
                assemblyFileStream << " $" << srcImm->getImmediate() << ",";
            }
            else if (auto srcReg =
                         std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                             src)) {
                assemblyFileStream << " %" << srcReg->getRegister() << ",";
            }
            else if (auto srcStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             src)) {
                assemblyFileStream << " " << srcStack->getOffset() << "(%rbp),";
            }
            if (auto dstReg =
                    std::dynamic_pointer_cast<Assembly::RegisterOperand>(dst)) {
                assemblyFileStream << " %" << dstReg->getRegister() << "\n";
            }
            else if (auto dstStack =
                         std::dynamic_pointer_cast<Assembly::StackOperand>(
                             dst)) {
                assemblyFileStream << " " << dstStack->getOffset()
                                   << "(%rbp)\n";
            }
        }
        else if (auto idivInstruction =
                     std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                         instruction)) {
            if (auto operand = idivInstruction->getOperand()) {
                if (auto regOperand =
                        std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                            operand)) {
                    assemblyFileStream << "    idivl %"
                                       << regOperand->getRegister() << "\n";
                }
                else if (auto stackOperand =
                             std::dynamic_pointer_cast<Assembly::StackOperand>(
                                 operand)) {
                    assemblyFileStream << "    idivl "
                                       << stackOperand->getOffset()
                                       << "(%rbp)\n";
                }
            }
        }
        else if (auto cdqInstruction =
                     std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                         instruction)) {
            assemblyFileStream << "    cdq\n";
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
