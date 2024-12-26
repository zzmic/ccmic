#include "fixupPass.h"

namespace Assembly {
void FixupPass::fixup(
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>> function,
    int stackSize) {
    auto instructions = function->at(0)->getFunctionBody();

    // Insert an allocate stack instruction at the beginning of the function.
    insertAllocateStackInstruction(instructions, stackSize);

    // Traverse the instructions and rewrite invalid mov instructions.
    for (auto it = instructions->begin(); it != instructions->end(); it++) {
        if (auto movInstr =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(*it)) {
            // If the mov instruction is invalid, rewrite it.
            // Replace the invalid mov instruction with two valid ones using
            // R10.
            // Replace the iterator with the new iterator returned by
            // `rewriteInvalidMov`.
            if (isInvalidMov(movInstr)) {
                it = rewriteInvalidMov(instructions, it, movInstr);
            }
        }
        else if (auto binInstr =
                     std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                         *it)) {
            if (isInvalidBinary(binInstr)) {
                it = rewriteInvalidBinary(instructions, it, binInstr);
            }
        }
        else if (auto idivInstr =
                     std::dynamic_pointer_cast<Assembly::IdivInstruction>(
                         *it)) {
            if (isInvalidIdiv(idivInstr)) {
                it = rewriteInvalidIdiv(instructions, it, idivInstr);
            }
        }
    }
}

void FixupPass::insertAllocateStackInstruction(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    int stackSize) {
    instructions->insert(instructions->begin(),
                         std::make_shared<AllocateStackInstruction>(stackSize));
}

bool FixupPass::isInvalidMov(
    std::shared_ptr<Assembly::MovInstruction> movInstr) {
    return std::dynamic_pointer_cast<Assembly::StackOperand>(
               movInstr->getSrc()) != nullptr &&
           std::dynamic_pointer_cast<Assembly::StackOperand>(
               movInstr->getDst()) != nullptr;
}

bool FixupPass::isInvalidBinary(
    std::shared_ptr<Assembly::BinaryInstruction> binInstr) {
    if (std::dynamic_pointer_cast<Assembly::AddOperator>(
            binInstr->getBinaryOperator()) != nullptr ||
        std::dynamic_pointer_cast<Assembly::SubtractOperator>(
            binInstr->getBinaryOperator()) != nullptr) {
        return std::dynamic_pointer_cast<Assembly::StackOperand>(
                   binInstr->getOperand1()) != nullptr &&
               std::dynamic_pointer_cast<Assembly::StackOperand>(
                   binInstr->getOperand2()) != nullptr;
    }
    else if (std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                 binInstr->getBinaryOperator()) != nullptr) {
        return std::dynamic_pointer_cast<Assembly::StackOperand>(
                   binInstr->getOperand2()) != nullptr;
    }
    return false;
}

bool FixupPass::isInvalidIdiv(
    std::shared_ptr<Assembly::IdivInstruction> idivInstr) {
    return std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
               idivInstr->getOperand()) != nullptr;
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMov(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovInstruction> movInst) {
    // Create a new register operand for R10 ("r10d").
    auto r10d = std::make_shared<Assembly::RegisterOperand>("r10d");

    // Create two new movInstructions using `r10d` as an intermediate register
    // based on the original source and destination operands.
    auto newMov1 =
        std::make_shared<Assembly::MovInstruction>(movInst->getSrc(), r10d);
    auto newMov2 =
        std::make_shared<Assembly::MovInstruction>(r10d, movInst->getDst());

    // Replace the original `mov` instruction with the first new `mov`
    // instruction.
    *it = newMov1;
    // Insert the second new `mov` instruction after the first one.
    it = instructions->insert(it + 1, newMov2);

    // Return the new iterator pointing to the second `mov` instruction.
    return it;
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidBinary(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::BinaryInstruction> binInstr) {
    if (std::dynamic_pointer_cast<Assembly::AddOperator>(
            binInstr->getBinaryOperator()) != nullptr ||
        std::dynamic_pointer_cast<Assembly::SubtractOperator>(
            binInstr->getBinaryOperator()) != nullptr) {
        auto r10d = std::make_shared<Assembly::RegisterOperand>("r10d");
        auto newMov = std::make_shared<Assembly::MovInstruction>(
            binInstr->getOperand1(), r10d);
        auto newBin = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), r10d, binInstr->getOperand2());
        *it = newMov;
        it = instructions->insert(it + 1, newBin);
    }
    else if (std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                 binInstr->getBinaryOperator()) != nullptr) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>("r11d");
        auto newMov1 = std::make_shared<Assembly::MovInstruction>(
            binInstr->getOperand2(), r11d);
        auto newImul = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getOperand1(), r11d);
        auto newMov2 = std::make_shared<Assembly::MovInstruction>(
            r11d, binInstr->getOperand2());
        *it = newMov1;
        it = instructions->insert(it + 1, newImul);
        it = instructions->insert(it + 1, newMov2);
    }

    return it;
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidIdiv(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::IdivInstruction> idivInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>("r10d");

    auto newMov = std::make_shared<Assembly::MovInstruction>(
        idivInstr->getOperand(), r10d);
    auto newIdiv = std::make_shared<Assembly::IdivInstruction>(r10d);

    *it = newMov;
    it = instructions->insert(it + 1, newIdiv);

    return it;
}

} // Namespace Assembly
