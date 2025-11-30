#include "fixupPass.h"
#include <limits>

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

// Function to insert an allocate-stack instruction at the beginning of a
// function.
// Rewrite: `FunctionDefinition(instructions)` ->
// `FunctionDefinition(instructions)` + `AllocateStack(stackSize)`.
void FixupPass::insertAllocateStackInstruction(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    int stackSize) {
    instructions->insert(instructions->begin(),
                         std::make_shared<BinaryInstruction>(
                             std::make_shared<SubtractOperator>(),
                             std::make_shared<Quadword>(),
                             std::make_shared<ImmediateOperand>(stackSize),
                             std::make_shared<Assembly::RegisterOperand>(
                                 std::make_shared<Assembly::SP>())));
}

// Function to rewrite a function definition.
// Rewrite: `FunctionDefinition(instructions)` ->
// `FunctionDefinition(instructions)` + `AllocateStack(stackSize)`.
void FixupPass::rewriteFunctionDefinition(
    std::shared_ptr<FunctionDefinition> functionDefinition) {
    auto instructions = functionDefinition->getFunctionBody();
    auto preAlignedStackSize = functionDefinition->getStackSize();
    // Align the stack size to the next multiple of 16.
    // Reference: https://math.stackexchange.com/a/291494.
    auto alignedStackSize = ((preAlignedStackSize - 1) | 15) + 1;
    // Insert an allocate-stack instruction at the beginning of each
    // function.
    insertAllocateStackInstruction(instructions,
                                   static_cast<int>(alignedStackSize));
    // Traverse the instructions (associated with (included in) the
    // function) and rewrite invalid instructions.
    for (auto it = instructions->begin(); it != instructions->end(); it++) {
        if (auto movInstr =
                std::dynamic_pointer_cast<Assembly::MovInstruction>(*it)) {
            // If the mov instruction is invalid, rewrite it.
            // Replace the invalid `mov` instruction with two valid ones using
            // R10.
            // Replace the iterator with the new iterator returned by
            // `rewriteInvalidMov`.
            if (isInvalidMov(movInstr)) {
                it = rewriteInvalidMov(instructions, it, movInstr);
            }
            // Check for large immediate values in mov instructions.
            else if (isInvalidLargeImmediateMov(movInstr)) {
                it =
                    rewriteInvalidLargeImmediateMov(instructions, it, movInstr);
            }
            // Check for 8-byte immediate values in movl instructions.
            else if (isInvalidLongwordImmediateMov(movInstr)) {
                it = rewriteInvalidLongwordImmediateMov(instructions, it,
                                                        movInstr);
            }
        }
        else if (auto movsxInstr =
                     std::dynamic_pointer_cast<Assembly::MovsxInstruction>(
                         *it)) {
            if (isInvalidMovsx(movsxInstr)) {
                it = rewriteInvalidMovsx(instructions, it, movsxInstr);
            }
        }
        else if (auto binInstr =
                     std::dynamic_pointer_cast<Assembly::BinaryInstruction>(
                         *it)) {
            if (isInvalidLargeImmediateBinary(binInstr)) {
                it = rewriteInvalidLargeImmediateBinary(instructions, it,
                                                        binInstr);
            }
            else if (isInvalidBinary(binInstr)) {
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
            if (isInvalidLargeImmediateCmp(cmpInstr)) {
                it =
                    rewriteInvalidLargeImmediateCmp(instructions, it, cmpInstr);
            }
            else if (isInvalidCmp(cmpInstr)) {
                it = rewriteInvalidCmp(instructions, it, cmpInstr);
            }
        }
        else if (auto pushInstr =
                     std::dynamic_pointer_cast<Assembly::PushInstruction>(
                         *it)) {
            if (isInvalidLargeImmediatePush(pushInstr)) {
                it = rewriteInvalidLargeImmediatePush(instructions, it,
                                                      pushInstr);
            }
        }
    }
}

// Function to check if a mov instruction is invalid.
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

// Function to check if a mov instruction has a large immediate value.
bool FixupPass::isInvalidLargeImmediateMov(
    std::shared_ptr<Assembly::MovInstruction> movInstr) {
    // Check if it's a quadword mov with large immediate to memory.
    auto quadwordType =
        std::dynamic_pointer_cast<Assembly::Quadword>(movInstr->getType());
    if (!quadwordType)
        return false;

    auto immediateSrc = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        movInstr->getSrc());
    if (!immediateSrc)
        return false;

    auto memoryDst =
        std::dynamic_pointer_cast<Assembly::StackOperand>(movInstr->getDst()) ||
        std::dynamic_pointer_cast<Assembly::DataOperand>(movInstr->getDst());
    if (!memoryDst)
        return false;

    // Check if immediate value is outside 32-bit signed range.
    long value = immediateSrc->getImmediateLong();
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

// Function to check if a mov instruction has a longword immediate value.
bool FixupPass::isInvalidLongwordImmediateMov(
    std::shared_ptr<Assembly::MovInstruction> movInstr) {
    // Check if it's a longword mov with 8-byte immediate value.
    auto longwordType =
        std::dynamic_pointer_cast<Assembly::Longword>(movInstr->getType());
    if (!longwordType)
        return false;

    auto immediateSrc = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        movInstr->getSrc());
    if (!immediateSrc)
        return false;

    // Check if immediate value is outside 32-bit range (truncation needed).
    long value = immediateSrc->getImmediateLong();
    return value > std::numeric_limits<unsigned int>::max() || value < 0;
}

// Function to check if a movsx instruction is invalid.
bool FixupPass::isInvalidMovsx(
    std::shared_ptr<Assembly::MovsxInstruction> movsxInstr) {
    // Movsx can't use a memory address as a destination or an immediate value
    // as a source.
    bool invalidSrc = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                          movsxInstr->getSrc()) != nullptr;
    bool invalidDst = std::dynamic_pointer_cast<Assembly::StackOperand>(
                          movsxInstr->getDst()) != nullptr ||
                      std::dynamic_pointer_cast<Assembly::DataOperand>(
                          movsxInstr->getDst()) != nullptr;
    return invalidSrc || invalidDst;
}

// Function to check if a binary instruction is invalid.
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

// Function to check if a binary instruction has a large immediate value.
bool FixupPass::isInvalidLargeImmediateBinary(
    std::shared_ptr<Assembly::BinaryInstruction> binInstr) {
    // Check if it's a `Quadword` `Binary` instruction with a large immediate
    // value.
    auto quadwordType =
        std::dynamic_pointer_cast<Assembly::Quadword>(binInstr->getType());
    if (!quadwordType)
        return false;

    auto immediateOp1 = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        binInstr->getOperand1());
    auto immediateOp2 = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        binInstr->getOperand2());

    std::shared_ptr<Assembly::ImmediateOperand> immediateOp = nullptr;
    if (immediateOp1) {
        immediateOp = immediateOp1;
    }
    else if (immediateOp2) {
        immediateOp = immediateOp2;
    }

    if (!immediateOp)
        return false;

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    long value = immediateOp->getImmediateLong();
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
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

// Function to check if a cmp instruction has a large immediate value.
bool FixupPass::isInvalidLargeImmediateCmp(
    std::shared_ptr<Assembly::CmpInstruction> cmpInstr) {
    // Check if it's a `Quadword` `Cmp` instruction with a large immediate
    // value.
    auto quadwordType =
        std::dynamic_pointer_cast<Assembly::Quadword>(cmpInstr->getType());
    if (!quadwordType)
        return false;

    auto immediateOp1 = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        cmpInstr->getOperand1());
    auto immediateOp2 = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        cmpInstr->getOperand2());

    std::shared_ptr<Assembly::ImmediateOperand> immediateOp = nullptr;
    if (immediateOp1) {
        immediateOp = immediateOp1;
    }
    else if (immediateOp2) {
        immediateOp = immediateOp2;
    }

    if (!immediateOp)
        return false;

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    long value = immediateOp->getImmediateLong();
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

// Function to check if a push instruction has a large immediate value.
bool FixupPass::isInvalidLargeImmediatePush(
    std::shared_ptr<Assembly::PushInstruction> pushInstr) {
    auto immediateOp = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        pushInstr->getOperand());
    if (!immediateOp)
        return false;

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    long value = immediateOp->getImmediateLong();
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

// Function to rewrite an invalid mov instruction.
// Rewrite: `Mov(Stack/Data, Stack/Data)` ->
// `Mov(Quadword, Stack/Data, Reg(R10))` + `Mov(Quadword, Reg(R10),
// Stack/Data)`.
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
    auto newMov1 = std::make_shared<Assembly::MovInstruction>(
        movInst->getType(), movInst->getSrc(), r10d);
    auto newMov2 = std::make_shared<Assembly::MovInstruction>(
        movInst->getType(), r10d, movInst->getDst());

    // Replace the original `mov` instruction with the first new `mov`
    // instruction.
    *it = newMov1;
    // Insert the second new `mov` instruction after the first one.
    it = instructions->insert(it + 1, newMov2);

    // Return the new iterator pointing to the second `mov` instruction.
    return it;
}

// Function to rewrite an invalid movsx instruction.
// Rewrite: `Movsx(Imm(large), Stack/Data)` ->
// `Mov(Longword, Imm(large), Reg(R10))` + `Movsx(Reg(R10), Reg(R11))` +
// `Mov(Quadword, Reg(R11), Stack/Data)`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMovsx(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovsxInstruction> movsxInst) {
    auto src = movsxInst->getSrc();
    auto dst = movsxInst->getDst();

    bool invalidSrc =
        std::dynamic_pointer_cast<Assembly::ImmediateOperand>(src) != nullptr;
    bool invalidDst =
        std::dynamic_pointer_cast<Assembly::StackOperand>(dst) != nullptr ||
        std::dynamic_pointer_cast<Assembly::DataOperand>(dst) != nullptr;

    if (invalidSrc && invalidDst) {
        auto r10d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R10>());
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());

        auto newMov1 = std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::Longword>(), src, r10d);

        auto newMovsx =
            std::make_shared<Assembly::MovsxInstruction>(r10d, r11d);

        auto newMov2 = std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::Quadword>(), r11d, dst);

        *it = newMov1;
        it = instructions->insert(it + 1, newMovsx);
        it = instructions->insert(it + 1, newMov2);
    }
    else if (invalidSrc) {
        auto r10d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R10>());

        auto newMov = std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::Longword>(), src, r10d);

        auto newMovsx = std::make_shared<Assembly::MovsxInstruction>(r10d, dst);

        *it = newMov;
        it = instructions->insert(it + 1, newMovsx);
    }
    else if (invalidDst) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());

        auto newMovsx = std::make_shared<Assembly::MovsxInstruction>(src, r11d);

        auto newMov = std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::Quadword>(), r11d, dst);

        *it = newMovsx;
        it = instructions->insert(it + 1, newMov);
    }

    return it;
}

// Function to rewrite an invalid binary instruction.
// Rewrite: `Binary(op, Stack/Data, Stack/Data)` ->
// `Mov(Quadword, Stack/Data, Reg(R10))` + `Binary(op, Quadword, Reg(R10),
// Stack/Data)`.
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
            binInstr->getType(), binInstr->getOperand1(), r10d);
        auto newBin = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getType(), r10d,
            binInstr->getOperand2());
        *it = newMov;
        it = instructions->insert(it + 1, newBin);
    }
    else if (std::dynamic_pointer_cast<Assembly::MultiplyOperator>(
                 binInstr->getBinaryOperator())) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto newMov1 = std::make_shared<Assembly::MovInstruction>(
            binInstr->getType(), binInstr->getOperand2(), r11d);
        auto newImul = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getType(),
            binInstr->getOperand1(), r11d);
        auto newMov2 = std::make_shared<Assembly::MovInstruction>(
            binInstr->getType(), r11d, binInstr->getOperand2());
        *it = newMov1;
        it = instructions->insert(it + 1, newImul);
        it = instructions->insert(it + 1, newMov2);
    }

    return it;
}

// Function to rewrite an invalid idiv instruction.
// Rewrite: `Idiv(Quadword, Imm(large))` ->
// `Mov(Quadword, Imm(large), Reg(R10))` + `Idiv(Quadword, Reg(R10))`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidIdiv(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::IdivInstruction> idivInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    auto newMov = std::make_shared<Assembly::MovInstruction>(
        idivInstr->getType(), idivInstr->getOperand(), r10d);
    auto newIdiv =
        std::make_shared<Assembly::IdivInstruction>(idivInstr->getType(), r10d);

    *it = newMov;
    it = instructions->insert(it + 1, newIdiv);

    return it;
}

// Function to rewrite an invalid cmp instruction.
// Rewrite: `Cmp(Stack/Data, Stack/Data)` ->
// `Mov(Quadword, Stack/Data, Reg(R10))` + `Cmp(Quadword, Reg(R10),
// Stack/Data)`.
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
            cmpInstr->getType(), cmpInstr->getOperand1(), r10d);
        auto newCmp = std::make_shared<Assembly::CmpInstruction>(
            cmpInstr->getType(), r10d, cmpInstr->getOperand2());
        *it = newMov;
        it = instructions->insert(it + 1, newCmp);
    }
    else if (std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                 cmpInstr->getOperand2())) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto newMov = std::make_shared<Assembly::MovInstruction>(
            cmpInstr->getType(), cmpInstr->getOperand2(), r11d);
        auto newCmp = std::make_shared<Assembly::CmpInstruction>(
            cmpInstr->getType(), cmpInstr->getOperand1(), r11d);
        *it = newMov;
        it = instructions->insert(it + 1, newCmp);
    }

    return it;
}

// Function to rewrite an invalid mov instruction with a quadword immediate
// value.
// Rewrite: `Mov(Quadword, Imm(large), Stack/Data)` ->
// `Mov(Quadword, Imm(large), Reg(R10))` + `Mov(Quadword, Reg(R10),
// Stack/Data)`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateMov(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovInstruction> movInst) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    auto newMov1 = std::make_shared<Assembly::MovInstruction>(
        movInst->getType(), movInst->getSrc(), r10d);
    auto newMov2 = std::make_shared<Assembly::MovInstruction>(
        movInst->getType(), r10d, movInst->getDst());

    *it = newMov1;
    it = instructions->insert(it + 1, newMov2);

    return it;
}

// Function to rewrite an invalid mov instruction with a longword immediate
// value.
// Rewrite: `Mov(Longword, Imm(large), Reg)` ->
// `Mov(Longword, Imm(truncated), Reg)`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLongwordImmediateMov(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions [[maybe_unused]],
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::MovInstruction> movInst) {
    auto immediateSrc = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        movInst->getSrc());
    long originalValue = immediateSrc->getImmediateLong();

    // Truncate the immediate value to 32 bits.
    long truncatedValue =
        static_cast<long>(static_cast<unsigned long>(originalValue) &
                          std::numeric_limits<unsigned int>::max());

    auto newImmediate =
        std::make_shared<Assembly::ImmediateOperand>(truncatedValue);
    auto newMov = std::make_shared<Assembly::MovInstruction>(
        movInst->getType(), newImmediate, movInst->getDst());

    *it = newMov;
    return it;
}

// Function to rewrite an invalid binary instruction with a large immediate
// value.
// Rewrite: `Binary(op, Quadword, Imm(large), Reg)` ->
// `Mov(Quadword, Imm(large), Reg(R10))` + `Binary(op, Quadword, Reg(R10),
// Reg)`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateBinary(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::BinaryInstruction> binInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    auto immediateOp = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        binInstr->getOperand1());
    auto otherOp = binInstr->getOperand2();
    bool isFirstOperand = true;
    if (!immediateOp) {
        immediateOp = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
            binInstr->getOperand2());
        otherOp = binInstr->getOperand1();
        isFirstOperand = false;
    }

    auto newMov = std::make_shared<Assembly::MovInstruction>(
        binInstr->getType(), immediateOp, r10d);

    // Check if `otherOp` is a memory operand (stack or data).
    // If it is, load it into a register first.
    auto otherStackOp =
        std::dynamic_pointer_cast<Assembly::StackOperand>(otherOp);
    auto otherDataOp =
        std::dynamic_pointer_cast<Assembly::DataOperand>(otherOp);
    std::shared_ptr<Assembly::RegisterOperand> otherRegOp = nullptr;
    if (otherStackOp || otherDataOp) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto loadOtherMov = std::make_shared<Assembly::MovInstruction>(
            binInstr->getType(), otherOp, r11d);
        *it = newMov;
        it = instructions->insert(it + 1, loadOtherMov);
        otherRegOp = r11d;
    }
    else {
        *it = newMov;
    }

    std::shared_ptr<Assembly::BinaryInstruction> newBin;
    std::shared_ptr<Assembly::Operand> finalOtherOp =
        otherRegOp ? std::static_pointer_cast<Assembly::Operand>(otherRegOp)
                   : otherOp;
    if (isFirstOperand) {
        newBin = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getType(), r10d,
            finalOtherOp);
    }
    else {
        newBin = std::make_shared<Assembly::BinaryInstruction>(
            binInstr->getBinaryOperator(), binInstr->getType(), finalOtherOp,
            r10d);
    }

    it = instructions->insert(it + 1, newBin);

    // If we loaded `otherOp` into a register, store it back to memory.
    if (otherRegOp) {
        auto storeMov = std::make_shared<Assembly::MovInstruction>(
            binInstr->getType(), otherRegOp, otherOp);
        it = instructions->insert(it + 1, storeMov);
    }

    return it;
}

// Function to rewrite an invalid cmp instruction with a large immediate
// value.
// Rewrite: `Cmp(Quadword, Imm(large), Reg)` ->
// `Mov(Quadword, Imm(large), Reg(R10))` + `Cmp(Quadword, Reg(R10), Reg)`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateCmp(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::CmpInstruction> cmpInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());

    auto immediateOp = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
        cmpInstr->getOperand1());
    auto otherOp = cmpInstr->getOperand2();
    bool isFirstOperand = true;
    if (!immediateOp) {
        immediateOp = std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
            cmpInstr->getOperand2());
        otherOp = cmpInstr->getOperand1();
        isFirstOperand = false;
    }

    auto newMov = std::make_shared<Assembly::MovInstruction>(
        cmpInstr->getType(), immediateOp, r10d);

    // Check if `otherOp` is also an immediate (even a small one).
    // If it is, load it into a register first.
    auto otherImmediateOp =
        std::dynamic_pointer_cast<Assembly::ImmediateOperand>(otherOp);
    std::shared_ptr<Assembly::RegisterOperand> otherRegOp = nullptr;
    if (otherImmediateOp) {
        auto r11d = std::make_shared<Assembly::RegisterOperand>(
            std::make_shared<Assembly::R11>());
        auto otherMov = std::make_shared<Assembly::MovInstruction>(
            cmpInstr->getType(), otherImmediateOp, r11d);
        *it = newMov;
        it = instructions->insert(it + 1, otherMov);
        otherRegOp = r11d;
    }
    else {
        *it = newMov;
    }

    std::shared_ptr<Assembly::CmpInstruction> newCmp;
    std::shared_ptr<Assembly::Operand> finalOtherOp =
        otherRegOp ? std::static_pointer_cast<Assembly::Operand>(otherRegOp)
                   : otherOp;
    if (isFirstOperand) {
        newCmp = std::make_shared<Assembly::CmpInstruction>(cmpInstr->getType(),
                                                            r10d, finalOtherOp);
    }
    else {
        newCmp = std::make_shared<Assembly::CmpInstruction>(cmpInstr->getType(),
                                                            finalOtherOp, r10d);
    }

    it = instructions->insert(it + 1, newCmp);

    return it;
}

// Function to rewrite an invalid push instruction with a large immediate
// value.
// Rewrite: `Push(Imm(large))` -> `Mov(Quadword, Imm(large), Reg(R10))` +
// `Push(Reg(R10))`.
std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediatePush(
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions,
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
    std::shared_ptr<Assembly::PushInstruction> pushInstr) {
    auto r10d = std::make_shared<Assembly::RegisterOperand>(
        std::make_shared<Assembly::R10>());
    auto newMov = std::make_shared<Assembly::MovInstruction>(
        std::make_shared<Assembly::Quadword>(), pushInstr->getOperand(), r10d);
    auto newPush = std::make_shared<Assembly::PushInstruction>(r10d);

    *it = newMov;
    it = instructions->insert(it + 1, newPush);

    return it;
}
} // Namespace Assembly
