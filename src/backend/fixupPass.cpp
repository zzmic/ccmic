#include "fixupPass.h"

namespace Assembly {
void FixupPass::fixup(
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels) {
    for (auto topLevel : *topLevels) {
        if (auto functionDefinition =
                std::dynamic_pointer_cast<FunctionDefinition>(topLevel)) {
            rewriteFunctionDefinition(functionDefinition);
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

void FixupPass::rewriteFunctionDefinition(
    std::shared_ptr<FunctionDefinition> functionDefinition) {
    auto instructions = functionDefinition->getFunctionBody();
    auto preAlignedStackSize = functionDefinition->getStackSize();
    // Align the stack size to the next multiple of 16.
    // Reference: https://math.stackexchange.com/a/291494.
    auto alignedStackSize = ((preAlignedStackSize - 1) | 15) + 1;
    // Insert an allocate-stack instruction at the beginning of each
    // function.
    insertAllocateStackInstruction(instructions, alignedStackSize);
    // Traverse the instructions (associated with (included in) the
    // function) and rewrite invalid instructions.
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
        else if (auto cmpInstr =
                     std::dynamic_pointer_cast<Assembly::CmpInstruction>(*it)) {
            if (isInvalidCmp(cmpInstr)) {
                it = rewriteInvalidCmp(instructions, it, cmpInstr);
            }
        }
    }
}

bool FixupPass::isInvalidMov(
    std::shared_ptr<Assembly::MovInstruction> movInstr) {
    // Stack operands and data operands are memory addresses (memory-address
    // operands).
    return (std::dynamic_pointer_cast<Assembly::StackOperand>(
                movInstr->getSrc()) != nullptr ||
            std::dynamic_pointer_cast<Assembly::DataOperand>(
                movInstr->getSrc()) != nullptr) &&
           (std::dynamic_pointer_cast<Assembly::StackOperand>(
                movInstr->getDst()) != nullptr ||
            std::dynamic_pointer_cast<Assembly::DataOperand>(
                movInstr->getDst()) != nullptr);
}

bool FixupPass::isInvalidBinary(
    std::shared_ptr<Assembly::BinaryInstruction> binInstr) {
    if (std::dynamic_pointer_cast<Assembly::AddOperator>(
            binInstr->getBinaryOperator()) ||
        std::dynamic_pointer_cast<Assembly::SubtractOperator>(
            binInstr->getBinaryOperator())) {
        return (std::dynamic_pointer_cast<Assembly::StackOperand>(
                    binInstr->getOperand1()) != nullptr ||
                std::dynamic_pointer_cast<Assembly::DataOperand>(
                    binInstr->getOperand1()) != nullptr) &&
               (std::dynamic_pointer_cast<Assembly::StackOperand>(
                    binInstr->getOperand2()) != nullptr ||
                std::dynamic_pointer_cast<Assembly::DataOperand>(
                    binInstr->getOperand2()) != nullptr);
    }
    else if (std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                 binInstr->getBinaryOperator())) {
        return std::dynamic_pointer_cast<Assembly::StackOperand>(
                   binInstr->getOperand2()) != nullptr ||
               std::dynamic_pointer_cast<Assembly::DataOperand>(
                   binInstr->getOperand2()) != nullptr;
    }
    return false;
}

bool FixupPass::isInvalidIdiv(
    std::shared_ptr<Assembly::IdivInstruction> idivInstr) {
    return std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
               idivInstr->getOperand()) != nullptr;
}

bool FixupPass::isInvalidCmp(
    std::shared_ptr<Assembly::CmpInstruction> cmpInstr) {
    if ((std::dynamic_pointer_cast<Assembly::StackOperand>(
             cmpInstr->getOperand1()) ||
         std::dynamic_pointer_cast<Assembly::DataOperand>(
             cmpInstr->getOperand1())) &&
        (std::dynamic_pointer_cast<Assembly::StackOperand>(
             cmpInstr->getOperand2()) ||
         std::dynamic_pointer_cast<Assembly::DataOperand>(
             cmpInstr->getOperand2()))) {
        return true;
    }
    else if (std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                 cmpInstr->getOperand2())) {
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMov(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovInstruction> movInst) {
    // Create a new register operand for R10 ("r10d").
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    // Create two new movInstructions using `r10d` as an intermediate register
    // based on the original source and destination operands.
    auto newMov1 =
        std::make_shared<Assembly::MovInstruction>(movInst->getSrc(), r10d);
    auto newMov2 = std::make_shared<Assembly::MovInstruction>(
        std::move(r10d), movInst->getDst());

    // Replace the original `mov` instruction with the first new `mov`
    // instruction.
    *it = std::move(newMov1);
    // Insert the second new `mov` instruction after the first one.
    it = instructions->insert(it + 1, std::move(newMov2));

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
            binInstr->getBinaryOperator()) ||
        std::dynamic_pointer_cast<Assembly::SubtractOperator>(
            binInstr->getBinaryOperator())) {
        auto r10d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R10>());
        auto newMov = std::make_shared<Assembly::MovInstruction>(
            binInstr->getOperand1(), r10d);
        auto newBin = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), std::move(r10d),
            binInstr->getOperand2());
        *it = std::move(newMov);
        it = instructions->insert(it + 1, std::move(newBin));
    }
    else if (std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                 binInstr->getBinaryOperator())) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto newMov1 = std::make_shared<Assembly::MovInstruction>(
            binInstr->getOperand2(), r11d);
        auto newImul = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getOperand1(), r11d);
        auto newMov2 = std::make_shared<Assembly::MovInstruction>(
            std::move(r11d), binInstr->getOperand2());
        *it = std::move(newMov1);
        it = instructions->insert(it + 1, std::move(newImul));
        it = instructions->insert(it + 1, std::move(newMov2));
    }

    return it;
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidIdiv(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::IdivInstruction> idivInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    auto newMov = std::make_shared<Assembly::MovInstruction>(
        idivInstr->getOperand(), r10d);
    auto newIdiv = std::make_shared<Assembly::IdivInstruction>(std::move(r10d));

    *it = std::move(newMov);
    it = instructions->insert(it + 1, std::move(newIdiv));

    return it;
}

// TODO(zzmic): Potentially revise the below.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidCmp(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::CmpInstruction> cmpInstr) {
    if ((std::dynamic_pointer_cast<Assembly::StackOperand>(
             cmpInstr->getOperand1()) ||
         std::dynamic_pointer_cast<Assembly::DataOperand>(
             cmpInstr->getOperand1())) &&
        (std::dynamic_pointer_cast<Assembly::StackOperand>(
             cmpInstr->getOperand2()) ||
         std::dynamic_pointer_cast<Assembly::DataOperand>(
             cmpInstr->getOperand2()))) {
        auto r10d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R10>());
        auto newMov = std::make_shared<Assembly::MovInstruction>(
            cmpInstr->getOperand1(), r10d);
        auto newCmp = std::make_shared<Assembly::CmpInstruction>(
            std::move(r10d), cmpInstr->getOperand2());
        *it = std::move(newMov);
        it = instructions->insert(it + 1, std::move(newCmp));
    }
    else if (std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                 cmpInstr->getOperand2())) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto newMov = std::make_shared<Assembly::MovInstruction>(
            cmpInstr->getOperand2(), r11d);
        auto newCmp = std::make_shared<Assembly::CmpInstruction>(
            cmpInstr->getOperand1(), std::move(r11d));
        *it = std::move(newMov);
        it = instructions->insert(it + 1, std::move(newCmp));
    }

    return it;
}
} // Namespace Assembly
