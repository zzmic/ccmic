#include "pseudoToStackPass.h"
#include <cassert>

namespace Assembly {
PseudoToStackPass::PseudoToStackPass(
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<AST::Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        symbols)
    : symbols(symbols) {}

void PseudoToStackPass::replacePseudoWithStackAndAssociateStackSize(
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &topLevels) {
    std::unordered_map<std::string, int> pseudoToStackMap;

    // Replace pseudo registers with stack operands in each instruction and
    // associate the stack size with each function.
    for (auto topLevel : *topLevels) {
        auto offset = -4;
        if (auto functionDefinition =
                std::dynamic_pointer_cast<Assembly::FunctionDefinition>(
                    topLevel)) {
            for (auto instruction : *functionDefinition->getFunctionBody()) {
                replacePseudoWithStack(instruction, pseudoToStackMap, offset);
            }
            functionDefinition->setStackSize(static_cast<size_t>(-offset));
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

void PseudoToStackPass::replacePseudoWithStack(
    std::shared_ptr<Assembly::Instruction> &instruction,
    std::unordered_map<std::string, int> &pseudoToStackMap, int &offset) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        auto src = movInstruction->getSrc();
        auto dst = movInstruction->getDst();
        replaceOperand(src, pseudoToStackMap, offset);
        replaceOperand(dst, pseudoToStackMap, offset);
        movInstruction->setSrc(src);
        movInstruction->setDst(dst);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        auto operand = unaryInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, offset);
        unaryInstruction->setOperand(operand);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                     instruction)) {
        auto operand1 = binaryInstruction->getOperand1();
        auto operand2 = binaryInstruction->getOperand2();
        replaceOperand(operand1, pseudoToStackMap, offset);
        replaceOperand(operand2, pseudoToStackMap, offset);
        binaryInstruction->setOperand1(operand1);
        binaryInstruction->setOperand2(operand2);
    }
    else if (auto cmpInstruction =
                 std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                     instruction)) {
        auto operand1 = cmpInstruction->getOperand1();
        auto operand2 = cmpInstruction->getOperand2();
        replaceOperand(operand1, pseudoToStackMap, offset);
        replaceOperand(operand2, pseudoToStackMap, offset);
        cmpInstruction->setOperand1(operand1);
        cmpInstruction->setOperand2(operand2);
    }
    else if (auto idivInstruction =
                 std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                     instruction)) {
        auto operand = idivInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, offset);
        idivInstruction->setOperand(operand);
    }
    else if (auto setCCInstruction =
                 std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                     instruction)) {
        auto operand = setCCInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, offset);
        setCCInstruction->setOperand(operand);
    }
    else if (auto pushInstruction =
                 std::dynamic_pointer_cast<Assembly::PushInstruction>(
                     instruction)) {
        auto operand = pushInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, offset);
        pushInstruction->setOperand(operand);
    }
}

void PseudoToStackPass::replaceOperand(
    std::shared_ptr<Assembly::Operand> &operand,
    std::unordered_map<std::string, int> &pseudoToStackMap, int &offset) {
    // If the operand is a pseudo register, replace it with a stack operand.
    if (auto pseudoOperand =
            std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                operand)) {
        const std::string &pseudoRegister = pseudoOperand->getPseudoRegister();
        if (pseudoToStackMap.find(pseudoRegister) == pseudoToStackMap.end()) {
            // If a pseudoregister is not in `pseudoToStackMap`, look it up in
            // the symbol table (first).
            if (symbols.find(pseudoRegister) != symbols.end()) {
                auto type = symbols[pseudoRegister].first;
                auto identifierAttribute = symbols[pseudoRegister].second;
                // If it has a static storage duration, map it to a data operand
                // by the same name.
                if (std::dynamic_pointer_cast<AST::StaticAttribute>(
                        identifierAttribute)) {
                    operand =
                        std::make_shared<Assembly::DataOperand>(pseudoRegister);
                    return;
                }
            }
            // Otherwise, assign it a new slot on the stack, as usual.
            // (Besides, if it is not in the symbol table, it is an IR
            // temporary.)
            pseudoToStackMap[pseudoRegister] = offset;
            offset -= 4;
        }
        operand = std::make_shared<Assembly::StackOperand>(
            pseudoToStackMap[pseudoRegister], std::make_shared<Assembly::BP>());
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
    }
}
} // namespace Assembly
