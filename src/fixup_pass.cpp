

#include "fixup_pass.h"

namespace Assembly {
void FixupPass::fixup(
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>> function,
    int stackSize) {
    auto instructions = function->at(0)->getFunctionBody();

    // Insert an allocate stack instruction at the beginning of the function.
    insertAllocateStack(instructions, stackSize);

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
    }
}

void FixupPass::insertAllocateStack(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    int stackSize) {
    instructions->insert(instructions->begin(),
                         std::make_shared<AllocateStack>(stackSize));
}

bool FixupPass::isInvalidMov(
    std::shared_ptr<Assembly::MovInstruction> movInstr) {
    return std::dynamic_pointer_cast<Assembly::StackOperand>(
               movInstr->getSrc()) &&
           std::dynamic_pointer_cast<Assembly::StackOperand>(
               movInstr->getDst());
}

std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMov(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovInstruction> movInst) {
    // Create a new register operand for R10 ("r10d").
    auto r10d = std::make_shared<Assembly::RegisterOperand>("r10d");

    // Obtain the original source and destination operands.
    auto src = movInst->getSrc();
    auto dst = movInst->getDst();

    // Create two new movInstructions using `r10d` as an intermediate register.
    auto newMov1 = std::make_shared<Assembly::MovInstruction>(src, r10d);
    auto newMov2 = std::make_shared<Assembly::MovInstruction>(r10d, dst);

    // Replace the original Mov instruction with the first new instruction.
    *it = newMov1;

    // Insert the second new instruction after the current one
    it = instructions->insert(it + 1, newMov2);

    // Return the new iterator pointing to the second new instruction.
    return it;
}

} // Namespace Assembly
