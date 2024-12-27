#include "prettyPrinters.h"

// Function to print the IR program onto the stdout.
void PrettyPrinters::printIRProgram(std::shared_ptr<IR::Program> irProgram) {
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
void PrettyPrinters::printAssemblyProgram(
    std::shared_ptr<Assembly::Program> assemblyProgram) {
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