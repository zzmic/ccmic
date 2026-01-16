#include "pseudoToStackPass.h"
#include "backendSymbolTable.h"
#include <cassert>

namespace Assembly {
void PseudoToStackPass::replacePseudoWithStackAndAssociateStackSize(
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &topLevels) {
    // Replace pseudo registers with stack operands in each instruction and
    // associate the stack size with each function.
    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<Assembly::FunctionDefinition>(
                    topLevel)) {
            // Reset offset and clear map for each function.
            this->offset = 0;
            this->pseudoToStackMap.clear();

            // Replace pseudo registers with stack operands in each instruction.
            for (auto instruction : *functionDefinition->getFunctionBody()) {
                replacePseudoWithStack(instruction);
            }

            // Set the stack size for the function.
            functionDefinition->setStackSize(
                static_cast<size_t>(-this->offset));
        }
    }

    // Check that all pseudo registers have been replaced.
    for (const auto &topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<Assembly::FunctionDefinition>(
                    topLevel)) {
            checkPseudoRegistersInFunctionDefinitionReplaced(
                functionDefinition);
        }
    }
}

std::shared_ptr<Assembly::Operand>
PseudoToStackPass::replaceOperand(std::shared_ptr<Assembly::Operand> operand) {
    if (!operand) {
        throw std::logic_error("Setting null operand in replaceOperand");
    }

    if (auto pseudoReg =
            std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                operand)) {
        const std::string &pseudoRegister = pseudoReg->getPseudoRegister();
        if (this->pseudoToStackMap.find(pseudoRegister) ==
            this->pseudoToStackMap.end()) {
            // If a pseudoregister is not in `pseudoToStackMap`, look it up in
            // the backend symbol table.
            if (backendSymbolTable.find(pseudoRegister) !=
                backendSymbolTable.end()) {
                auto backendEntry = backendSymbolTable[pseudoRegister];
                // If it has a static storage duration, map it to a data operand
                // by the same name.
                if (auto objEntry =
                        std::dynamic_pointer_cast<ObjEntry>(backendEntry)) {
                    if (objEntry->isStaticStorage()) {
                        return std::make_shared<Assembly::DataOperand>(
                            pseudoRegister);
                    }
                }
            }
            // Otherwise, assign it a new slot on the stack based on its type.
            // Look up the type in the backend symbol table to determine the
            // allocation size.
            int allocationSize =
                8; // Default to 8 bytes for temporary variables
            if (backendSymbolTable.find(pseudoRegister) !=
                backendSymbolTable.end()) {
                auto backendEntry = backendSymbolTable[pseudoRegister];
                if (auto objEntry =
                        std::dynamic_pointer_cast<ObjEntry>(backendEntry)) {
                    auto assemblyType = objEntry->getAssemblyType();
                    if (std::dynamic_pointer_cast<Quadword>(assemblyType)) {
                        allocationSize = 8; // 8 bytes for `Quadword`.
                    }
                    else {
                        allocationSize = 4; // 4 bytes for `Longword`.
                    }
                }
            }

            // For `Quadword` types, ensure 8-byte alignment (as required by the
            // System V ABI).
            if (allocationSize == 8) {
                // Round down to the next multiple of 8 for negative offsets.
                int rem = this->offset % 8;
                if (rem != 0) {
                    this->offset -= (8 + rem);
                }
            }

            // Update the offset to the next available stack slot.
            this->offset -= allocationSize;
            // Update the `pseudoToStackMap` with the new offset.
            this->pseudoToStackMap[pseudoRegister] = this->offset;
        }

        // Replace the pseudo register with a stack operand.
        return std::make_shared<Assembly::StackOperand>(
            this->pseudoToStackMap[pseudoRegister],
            std::make_shared<Assembly::BP>());
    }

    // If not a PseudoRegisterOperand, return as is
    return operand;
}

void PseudoToStackPass::replacePseudoWithStack(
    std::shared_ptr<Assembly::Instruction> &instruction) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        auto src = movInstruction->getSrc();
        auto dst = movInstruction->getDst();
        auto newSrc = replaceOperand(src);
        auto newDst = replaceOperand(dst);
        movInstruction->setSrc(newSrc);
        movInstruction->setDst(newDst);
    }
    else if (auto movsxInstruction =
                 std::dynamic_pointer_cast<Assembly::MovsxInstruction>(
                     instruction)) {
        auto src = movsxInstruction->getSrc();
        auto dst = movsxInstruction->getDst();
        auto newSrc = replaceOperand(src);
        auto newDst = replaceOperand(dst);
        movsxInstruction->setSrc(newSrc);
        movsxInstruction->setDst(newDst);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        auto operand = unaryInstruction->getOperand();
        auto newOperand = replaceOperand(operand);
        unaryInstruction->setOperand(newOperand);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                     instruction)) {
        auto operand1 = binaryInstruction->getOperand1();
        auto operand2 = binaryInstruction->getOperand2();
        auto newOperand1 = replaceOperand(operand1);
        auto newOperand2 = replaceOperand(operand2);
        binaryInstruction->setOperand1(newOperand1);
        binaryInstruction->setOperand2(newOperand2);
    }
    else if (auto cmpInstruction =
                 std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                     instruction)) {
        auto operand1 = cmpInstruction->getOperand1();
        auto operand2 = cmpInstruction->getOperand2();
        auto newOperand1 = replaceOperand(operand1);
        auto newOperand2 = replaceOperand(operand2);
        cmpInstruction->setOperand1(newOperand1);
        cmpInstruction->setOperand2(newOperand2);
    }
    else if (auto idivInstruction =
                 std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                     instruction)) {
        auto operand = idivInstruction->getOperand();
        auto newOperand = replaceOperand(operand);
        idivInstruction->setOperand(newOperand);
    }
    else if (auto setCCInstruction =
                 std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                     instruction)) {
        auto operand = setCCInstruction->getOperand();
        auto newOperand = replaceOperand(operand);
        setCCInstruction->setOperand(newOperand);
    }
    else if (auto pushInstruction =
                 std::dynamic_pointer_cast<Assembly::PushInstruction>(
                     instruction)) {
        auto operand = pushInstruction->getOperand();
        auto newOperand = replaceOperand(operand);
        pushInstruction->setOperand(newOperand);
    }
    else if (auto retInstruction =
                 std::dynamic_pointer_cast<Assembly::RetInstruction>(
                     instruction)) {
        (void)retInstruction;
    }
    else if (auto callInstruction =
                 std::dynamic_pointer_cast<Assembly::CallInstruction>(
                     instruction)) {
        (void)callInstruction;
    }
    else if (auto cdqInstruction =
                 std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                     instruction)) {
        (void)cdqInstruction;
    }
    else if (auto jmpInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpInstruction>(
                     instruction)) {
        (void)jmpInstruction;
    }
    else if (auto jmpCCInstruction =
                 std::dynamic_pointer_cast<Assembly::JmpCCInstruction>(
                     instruction)) {
        (void)jmpCCInstruction;
    }
    else if (auto labelInstruction =
                 std::dynamic_pointer_cast<Assembly::LabelInstruction>(
                     instruction)) {
        (void)labelInstruction;
    }
    else {
        throw std::logic_error("Unsupported instruction type while replacing "
                               "pseudo registers with stack operands");
    }
}

void PseudoToStackPass::checkPseudoRegistersInFunctionDefinitionReplaced(
    std::shared_ptr<Assembly::FunctionDefinition> functionDefinition) {
    for (const auto &instruction : *functionDefinition->getFunctionBody()) {
        if (auto movInstruction =
                std::dynamic_pointer_cast<MovInstruction>(instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                movInstruction->getSrc()));
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                movInstruction->getDst()));
        }
        else if (auto movsxInstruction =
                     std::dynamic_pointer_cast<Assembly::MovsxInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                movsxInstruction->getSrc()));
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                movsxInstruction->getDst()));
        }
        else if (auto unaryInstruction =
                     std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                unaryInstruction->getOperand()));
        }
        else if (auto binaryInstruction =
                     std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                binaryInstruction->getOperand1()));
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                binaryInstruction->getOperand2()));
        }
        else if (auto cmpInstruction =
                     std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                cmpInstruction->getOperand1()));
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                cmpInstruction->getOperand2()));
        }
        else if (auto idivInstruction =
                     std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                idivInstruction->getOperand()));
        }
        else if (auto setCCInstruction =
                     std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                setCCInstruction->getOperand()));
        }
        else if (auto pushInstruction =
                     std::dynamic_pointer_cast<Assembly::PushInstruction>(
                         instruction)) {
            assert(!std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                pushInstruction->getOperand()));
        }
        else if (auto retInstruction =
                     std::dynamic_pointer_cast<Assembly::RetInstruction>(
                         instruction)) {
            // RetInstruction doesn't have operands to check
            (void)retInstruction; // Suppress unused variable warning
        }
        else if (auto callInstruction =
                     std::dynamic_pointer_cast<Assembly::CallInstruction>(
                         instruction)) {
            // CallInstruction doesn't have operands to check
            (void)callInstruction; // Suppress unused variable warning
        }
        else if (auto cdqInstruction =
                     std::dynamic_pointer_cast<Assembly::CdqInstruction>(
                         instruction)) {
            // CdqInstruction doesn't have operands to check
            (void)cdqInstruction; // Suppress unused variable warning
        }
        else if (auto jmpInstruction =
                     std::dynamic_pointer_cast<Assembly::JmpInstruction>(
                         instruction)) {
            // JmpInstruction doesn't have operands to check
            (void)jmpInstruction; // Suppress unused variable warning
        }
        else if (auto jmpCCInstruction =
                     std::dynamic_pointer_cast<Assembly::JmpCCInstruction>(
                         instruction)) {
            // JmpCCInstruction doesn't have operands to check
            (void)jmpCCInstruction; // Suppress unused variable warning
        }
        else if (auto labelInstruction =
                     std::dynamic_pointer_cast<Assembly::LabelInstruction>(
                         instruction)) {
            // LabelInstruction doesn't have operands to check
            (void)labelInstruction; // Suppress unused variable warning
        }
        else {
            throw std::logic_error("Unsupported instruction type while "
                                   "checking whether pseudo registers have "
                                   "been replaced in function definition");
        }
    }
}
} // namespace Assembly
