#include "pipelineStagesExecutors.h"

std::vector<Token>
PipelineStagesExecutors::lexerExecutor(const std::string &sourceFile) {
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
PipelineStagesExecutors::parserExecutor(const std::vector<Token> &tokens) {
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
std::shared_ptr<IR::Program> PipelineStagesExecutors::irGeneratorExecutor(
    std::shared_ptr<AST::Program> astProgram) {
    std::cout << "\n";
    // Instantiate an IR generator object and generate the IR.
    IRGenerator irGenerator;
    return irGenerator.generate(astProgram);
}

// Function to generate (but not yet emit) the assembly program from the AST
// program.
std::shared_ptr<Assembly::Program> PipelineStagesExecutors::codegenExecutor(
    std::shared_ptr<IR::Program> irProgram) {
    // Instantiate an assembly generator object and generate the assembly.
    Assembly::AssemblyGenerator assemblyGenerator;
    std::shared_ptr<Assembly::Program> assemblyProgram =
        assemblyGenerator.generate(irProgram);

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

// Function to emit the generated assembly code to the assembly file.
void PipelineStagesExecutors::codeEmissionExecutor(
    std::shared_ptr<Assembly::Program> assemblyProgram,
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
