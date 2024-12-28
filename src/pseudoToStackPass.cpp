#include "pseudoToStackPass.h"
#include <cassert>

namespace Assembly {
int PseudoToStackPass::replacePseudoWithStackAndReturnOffset(
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
        &function) {
    std::unordered_map<std::string, int> pseudoToStackMap;
    int currentOffset = 0;

    auto instructions = function->at(0)->getFunctionBody();
    for (auto &instruction : *instructions) {
        replacePseudoWithStack(instruction, pseudoToStackMap, currentOffset);
    }

    // Check that all pseudo registers have been replaced.
    for (const auto &instruction : *function->at(0)->getFunctionBody()) {
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
    }

    return -currentOffset;
}

void PseudoToStackPass::replacePseudoWithStack(
    std::shared_ptr<Assembly::Instruction> &instruction,
    std::unordered_map<std::string, int> &pseudoToStackMap,
    int &currentOffset) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        auto src = movInstruction->getSrc();
        auto dst = movInstruction->getDst();
        replaceOperand(src, pseudoToStackMap, currentOffset);
        replaceOperand(dst, pseudoToStackMap, currentOffset);
        movInstruction->setSrc(src);
        movInstruction->setDst(dst);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        auto operand = unaryInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, currentOffset);
        unaryInstruction->setOperand(operand);
    }
    else if (auto binaryInstruction =
                 std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                     instruction)) {
        auto operand1 = binaryInstruction->getOperand1();
        auto operand2 = binaryInstruction->getOperand2();
        replaceOperand(operand1, pseudoToStackMap, currentOffset);
        replaceOperand(operand2, pseudoToStackMap, currentOffset);
        binaryInstruction->setOperand1(operand1);
        binaryInstruction->setOperand2(operand2);
    }
    else if (auto cmpInstruction =
                 std::dynamic_pointer_cast<Assembly::CmpInstruction>(
                     instruction)) {
        auto operand1 = cmpInstruction->getOperand1();
        auto operand2 = cmpInstruction->getOperand2();
        replaceOperand(operand1, pseudoToStackMap, currentOffset);
        replaceOperand(operand2, pseudoToStackMap, currentOffset);
        cmpInstruction->setOperand1(operand1);
        cmpInstruction->setOperand2(operand2);
    }
    else if (auto idivInstruction =
                 std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                     instruction)) {
        auto operand = idivInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, currentOffset);
        idivInstruction->setOperand(operand);
    }
    else if (auto setCCInstruction =
                 std::dynamic_pointer_cast<Assembly::SetCCInstruction>(
                     instruction)) {
        auto operand = setCCInstruction->getOperand();
        replaceOperand(operand, pseudoToStackMap, currentOffset);
        setCCInstruction->setOperand(operand);
    }
}

void PseudoToStackPass::replaceOperand(
    std::shared_ptr<Assembly::Operand> &operand,
    std::unordered_map<std::string, int> &pseudoToStackMap,
    int &currentOffset) {
    // If the operand is a pseudo register, replace it with a stack operand.
    if (auto pseudoOperand =
            std::dynamic_pointer_cast<Assembly::PseudoRegisterOperand>(
                operand)) {
        const std::string &pseudoRegister = pseudoOperand->getPseudoRegister();

        if (pseudoToStackMap.find(pseudoRegister) == pseudoToStackMap.end()) {
            pseudoToStackMap[pseudoRegister] = currentOffset;
            currentOffset -= 4;
        }

        operand = std::make_shared<Assembly::StackOperand>(
            pseudoToStackMap[pseudoRegister]);
    }
}
} // namespace Assembly
