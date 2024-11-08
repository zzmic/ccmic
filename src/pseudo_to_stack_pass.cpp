#include "pseudo_to_stack_pass.h"

namespace Assembly {
int PseudoToStackPass::returnOffset(
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
        function) {
    std::unordered_map<std::string, int> pseudoToStackMap;
    int currentOffset = 0;

    auto instructions = function->at(0)->getFunctionBody();
    for (auto &instruction : *instructions) {
        replacePseudoWithStack(instruction, pseudoToStackMap, currentOffset);
    }

    return -currentOffset;
}

void PseudoToStackPass::replacePseudoWithStack(
    std::shared_ptr<Assembly::Instruction> instruction,
    std::unordered_map<std::string, int> &pseudoToStackMap,
    int &currentOffset) {
    if (auto movInstruction =
            std::dynamic_pointer_cast<Assembly::MovInstruction>(instruction)) {
        replaceOperand(movInstruction->getSrc(), pseudoToStackMap,
                       currentOffset);
        replaceOperand(movInstruction->getDst(), pseudoToStackMap,
                       currentOffset);
    }
    else if (auto unaryInstruction =
                 std::dynamic_pointer_cast<Assembly::UnaryInstruction>(
                     instruction)) {
        replaceOperand(unaryInstruction->getOperand(), pseudoToStackMap,
                       currentOffset);
    }
}

void PseudoToStackPass::replaceOperand(
    std::shared_ptr<Assembly::Operand> operand,
    std::unordered_map<std::string, int> &pseudoToStackMap,
    int &currentOffset) {
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
