#include "fixupPass.h"
#include "../utils/constants.h"
#include "assembly.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

/**
 * Unnamed namespace for helper functions for the fixup pass.
 */
namespace {
/**
 * Clone an assembly type.
 *
 * @param type The assembly type to clone.
 * @return The cloned assembly type.
 */
std::unique_ptr<Assembly::AssemblyType>
cloneAssemblyType(const Assembly::AssemblyType *type) {
    if (type == nullptr) {
        throw std::logic_error(
            "Cloning null AssemblyType in cloneAssemblyType");
    }
    if (dynamic_cast<const Assembly::Longword *>(type) != nullptr) {
        return std::make_unique<Assembly::Longword>();
    }
    else if (dynamic_cast<const Assembly::Quadword *>(type) != nullptr) {
        return std::make_unique<Assembly::Quadword>();
    }
    const auto &r = *type;
    throw std::logic_error("Unsupported AssemblyType in cloneAssemblyType: " +
                           std::string(typeid(r).name()));
}

/**
 * Clone an assembly operand.
 *
 * @param operand The assembly operand to clone.
 * @return The cloned assembly operand.
 */
std::unique_ptr<Assembly::Operand>
cloneOperand(const Assembly::Operand *operand) {
    if (operand == nullptr) {
        throw std::logic_error("Cloning null Operand in cloneOperand");
    }
    if (const auto *immOp =
            dynamic_cast<const Assembly::ImmediateOperand *>(operand)) {
        return std::make_unique<Assembly::ImmediateOperand>(
            immOp->getImmediate());
    }
    else if (const auto *regOp =
                 dynamic_cast<const Assembly::RegisterOperand *>(operand)) {
        auto *reg = regOp->getRegister();
        if (dynamic_cast<const Assembly::AX *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::AX>());
        }
        if (dynamic_cast<const Assembly::CX *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::CX>());
        }
        if (dynamic_cast<const Assembly::DX *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::DX>());
        }
        if (dynamic_cast<const Assembly::DI *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::DI>());
        }
        if (dynamic_cast<const Assembly::SI *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::SI>());
        }
        if (dynamic_cast<const Assembly::R8 *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R8>());
        }
        if (dynamic_cast<const Assembly::R9 *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R9>());
        }
        if (dynamic_cast<const Assembly::R10 *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>());
        }
        if (dynamic_cast<const Assembly::R11 *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>());
        }
        if (dynamic_cast<const Assembly::SP *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::SP>());
        }
        if (dynamic_cast<const Assembly::BP *>(reg) != nullptr) {
            return std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::BP>());
        }
        const auto &r = *reg;
        throw std::logic_error("Unsupported register in cloneOperand: " +
                               std::string(typeid(r).name()));
    }
    else if (const auto *stackOp =
                 dynamic_cast<const Assembly::StackOperand *>(operand)) {
        auto *reservedReg = stackOp->getReservedRegister();
        if (dynamic_cast<const Assembly::SP *>(reservedReg) != nullptr) {
            return std::make_unique<Assembly::StackOperand>(
                stackOp->getOffset(), std::make_unique<Assembly::SP>());
        }
        if (dynamic_cast<const Assembly::BP *>(reservedReg) != nullptr) {
            return std::make_unique<Assembly::StackOperand>(
                stackOp->getOffset(), std::make_unique<Assembly::BP>());
        }
        const auto &r = *reservedReg;
        throw std::logic_error(
            "Unsupported reserved register in cloneOperand: " +
            std::string(typeid(r).name()));
    }
    else if (const auto *dataOp =
                 dynamic_cast<const Assembly::DataOperand *>(operand)) {
        return std::make_unique<Assembly::DataOperand>(dataOp->getIdentifier());
    }
    else if (const auto *pseudoOp =
                 dynamic_cast<const Assembly::PseudoRegisterOperand *>(
                     operand)) {
        return std::make_unique<Assembly::PseudoRegisterOperand>(
            pseudoOp->getPseudoRegister());
    }
    const auto &r = *operand;
    throw std::logic_error("Unsupported Operand in cloneOperand: " +
                           std::string(typeid(r).name()));
}

/**
 * Clone an assembly binary operator.
 *
 * @param binaryOperator The assembly binary operator to clone.
 * @return The cloned assembly binary operator.
 */
std::unique_ptr<Assembly::BinaryOperator>
cloneBinaryOperator(const Assembly::BinaryOperator *binaryOperator) {
    if (binaryOperator == nullptr) {
        throw std::logic_error(
            "Cloning null BinaryOperator in cloneBinaryOperator");
    }
    if (dynamic_cast<const Assembly::AddOperator *>(binaryOperator) !=
        nullptr) {
        return std::make_unique<Assembly::AddOperator>();
    }
    else if (dynamic_cast<const Assembly::SubtractOperator *>(binaryOperator) !=
             nullptr) {
        return std::make_unique<Assembly::SubtractOperator>();
    }
    else if (dynamic_cast<const Assembly::MultiplyOperator *>(binaryOperator) !=
             nullptr) {
        return std::make_unique<Assembly::MultiplyOperator>();
    }
    const auto &r = *binaryOperator;
    throw std::logic_error(
        "Unsupported BinaryOperator in cloneBinaryOperator: " +
        std::string(typeid(r).name()));
}
} // namespace

namespace Assembly {
void FixupPass::fixup(std::vector<std::unique_ptr<TopLevel>> &topLevels) {
    for (auto &topLevel : topLevels) {
        if (auto *functionDefinition =
                dynamic_cast<FunctionDefinition *>(topLevel.get())) {
            rewriteFunctionDefinition(*functionDefinition);
        }
    }
}

void FixupPass::insertAllocateStackInstruction(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    int stackSize) {
    instructions.insert(instructions.begin(),
                        std::make_unique<BinaryInstruction>(
                            std::make_unique<SubtractOperator>(),
                            std::make_unique<Quadword>(),
                            std::make_unique<ImmediateOperand>(stackSize),
                            std::make_unique<Assembly::RegisterOperand>(
                                std::make_unique<Assembly::SP>())));
}

void FixupPass::rewriteFunctionDefinition(
    FunctionDefinition &functionDefinition) {
    auto &instructions = functionDefinition.getFunctionBody();
    auto preAlignedStackSize = functionDefinition.getStackSize();
    // Align the stack size to the next multiple of 16.
    // Reference: <https://math.stackexchange.com/a/291494>.
    auto alignedStackSize =
        ((preAlignedStackSize - 1) | STACK_ALIGNMENT_MASK) + 1;
    // Insert an allocate-stack instruction at the beginning of each
    // function.
    insertAllocateStackInstruction(instructions,
                                   static_cast<int>(alignedStackSize));
    // Traverse the instructions (associated with (included in) the
    // function) and rewrite invalid instructions.
    for (auto it = instructions.begin(); it != instructions.end(); ++it) {
        if (auto *movInstr =
                dynamic_cast<Assembly::MovInstruction *>(it->get())) {
            // If the mov instruction is invalid, rewrite it.
            // Replace the invalid `mov` instruction with two valid ones using
            // R10.
            // Replace the iterator with the new iterator returned by
            // `rewriteInvalidMov`.
            if (isInvalidMov(*movInstr)) {
                it = rewriteInvalidMov(instructions, it, *movInstr);
            }
            // Check for large immediate values in mov instructions.
            else if (isInvalidLargeImmediateMov(*movInstr)) {
                it = rewriteInvalidLargeImmediateMov(instructions, it,
                                                     *movInstr);
            }
            // Check for 8-byte immediate values in movl instructions.
            else if (isInvalidLongwordImmediateMov(*movInstr)) {
                it = rewriteInvalidLongwordImmediateMov(instructions, it,
                                                        *movInstr);
            }
        }
        else if (auto *movsxInstr =
                     dynamic_cast<Assembly::MovsxInstruction *>(it->get())) {
            if (isInvalidMovsx(*movsxInstr)) {
                it = rewriteInvalidMovsx(instructions, it, *movsxInstr);
            }
        }
        else if (auto *binInstr =
                     dynamic_cast<Assembly::BinaryInstruction *>(it->get())) {
            if (isInvalidLargeImmediateBinary(*binInstr)) {
                it = rewriteInvalidLargeImmediateBinary(instructions, it,
                                                        *binInstr);
            }
            else if (isInvalidBinary(*binInstr)) {
                it = rewriteInvalidBinary(instructions, it, *binInstr);
            }
        }
        else if (auto *idivInstr =
                     dynamic_cast<Assembly::IdivInstruction *>(it->get())) {
            if (isInvalidIdiv(*idivInstr)) {
                it = rewriteInvalidIdiv(instructions, it, *idivInstr);
            }
        }
        else if (auto *divInstr =
                     dynamic_cast<Assembly::DivInstruction *>(it->get())) {
            if (isInvalidDiv(*divInstr)) {
                it = rewriteInvalidDiv(instructions, it, *divInstr);
            }
        }
        else if (auto *movZeroExtendInstr =
                     dynamic_cast<Assembly::MovZeroExtendInstruction *>(
                         it->get())) {
            it = rewriteMovZeroExtend(instructions, it, *movZeroExtendInstr);
        }
        else if (auto *cmpInstr =
                     dynamic_cast<Assembly::CmpInstruction *>(it->get())) {
            if (isInvalidLargeImmediateCmp(*cmpInstr)) {
                it = rewriteInvalidLargeImmediateCmp(instructions, it,
                                                     *cmpInstr);
            }
            else if (isInvalidCmp(*cmpInstr)) {
                it = rewriteInvalidCmp(instructions, it, *cmpInstr);
            }
        }
        else if (auto *pushInstr =
                     dynamic_cast<Assembly::PushInstruction *>(it->get())) {
            if (isInvalidLargeImmediatePush(*pushInstr)) {
                it = rewriteInvalidLargeImmediatePush(instructions, it,
                                                      *pushInstr);
            }
        }
    }
}

bool FixupPass::isInvalidMov(const Assembly::MovInstruction &movInstr) {
    // Stack operands and data operands are memory addresses (memory-address
    // operands).
    const auto *src = movInstr.getSrc();
    const auto *dst = movInstr.getDst();
    return (dynamic_cast<const Assembly::StackOperand *>(src) != nullptr ||
            dynamic_cast<const Assembly::DataOperand *>(src) != nullptr) &&
           (dynamic_cast<const Assembly::StackOperand *>(dst) != nullptr ||
            dynamic_cast<const Assembly::DataOperand *>(dst) != nullptr);
}

bool FixupPass::isInvalidLargeImmediateMov(
    const Assembly::MovInstruction &movInstr) {
    // Check if it's a quadword mov with large immediate to memory.
    const auto *quadwordType =
        dynamic_cast<const Assembly::Quadword *>(movInstr.getType());
    if (quadwordType == nullptr) {
        return false;
    }

    const auto *immediateSrc =
        dynamic_cast<const Assembly::ImmediateOperand *>(movInstr.getSrc());
    if (immediateSrc == nullptr) {
        return false;
    }

    auto memoryDst =
        (dynamic_cast<const Assembly::StackOperand *>(movInstr.getDst()) !=
         nullptr) ||
        (dynamic_cast<const Assembly::DataOperand *>(movInstr.getDst()) !=
         nullptr);
    if (!memoryDst) {
        return false;
    }

    // Check if immediate value is outside 32-bit signed range.
    // We interpret the unsigned long as a signed long for range checking.
    auto value = static_cast<long>(immediateSrc->getImmediate());
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

bool FixupPass::isInvalidLongwordImmediateMov(
    const Assembly::MovInstruction &movInstr) {
    // Check if it's a longword mov with 8-byte immediate value.
    const auto *longwordType =
        dynamic_cast<const Assembly::Longword *>(movInstr.getType());
    if (longwordType == nullptr) {
        return false;
    }

    const auto *immediateSrc =
        dynamic_cast<const Assembly::ImmediateOperand *>(movInstr.getSrc());
    if (immediateSrc == nullptr) {
        return false;
    }

    // Check if immediate value is outside 32-bit unsigned range (truncation
    // needed).
    auto value = immediateSrc->getImmediate();
    return value > std::numeric_limits<unsigned int>::max();
}

bool FixupPass::isInvalidMovsx(const Assembly::MovsxInstruction &movsxInstr) {
    // Movsx can't use a memory address as a destination or an immediate value
    // as a source.
    const bool invalidSrc = dynamic_cast<const Assembly::ImmediateOperand *>(
                                movsxInstr.getSrc()) != nullptr;
    const bool invalidDst =
        dynamic_cast<const Assembly::StackOperand *>(movsxInstr.getDst()) !=
            nullptr ||
        dynamic_cast<const Assembly::DataOperand *>(movsxInstr.getDst()) !=
            nullptr;
    return invalidSrc || invalidDst;
}

bool FixupPass::isInvalidBinary(const Assembly::BinaryInstruction &binInstr) {
    if ((dynamic_cast<const Assembly::AddOperator *>(
             binInstr.getBinaryOperator()) != nullptr) ||
        (dynamic_cast<const Assembly::SubtractOperator *>(
             binInstr.getBinaryOperator()) != nullptr)) {
        return (dynamic_cast<const Assembly::StackOperand *>(
                    binInstr.getOperand1()) != nullptr ||
                dynamic_cast<const Assembly::DataOperand *>(
                    binInstr.getOperand1()) != nullptr) &&
               (dynamic_cast<const Assembly::StackOperand *>(
                    binInstr.getOperand2()) != nullptr ||
                dynamic_cast<const Assembly::DataOperand *>(
                    binInstr.getOperand2()) != nullptr);
    }
    else if (dynamic_cast<const Assembly::MultiplyOperator *>(
                 binInstr.getBinaryOperator()) != nullptr) {
        return dynamic_cast<const Assembly::StackOperand *>(
                   binInstr.getOperand2()) != nullptr ||
               dynamic_cast<const Assembly::DataOperand *>(
                   binInstr.getOperand2()) != nullptr;
    }
    return false;
}

bool FixupPass::isInvalidLargeImmediateBinary(
    const Assembly::BinaryInstruction &binInstr) {
    // Check if it's a `Quadword` `Binary` instruction with a large immediate
    // value.
    const auto *quadwordType =
        dynamic_cast<const Assembly::Quadword *>(binInstr.getType());
    if (quadwordType == nullptr) {
        return false;
    }

    const auto *immediateOp1 = dynamic_cast<const Assembly::ImmediateOperand *>(
        binInstr.getOperand1());
    const auto *immediateOp2 = dynamic_cast<const Assembly::ImmediateOperand *>(
        binInstr.getOperand2());

    const Assembly::ImmediateOperand *immediateOp = nullptr;
    if (immediateOp1 != nullptr) {
        immediateOp = immediateOp1;
    }
    else if (immediateOp2 != nullptr) {
        immediateOp = immediateOp2;
    }

    if (immediateOp == nullptr) {
        return false;
    }

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    const long value = static_cast<long>(immediateOp->getImmediate());
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

bool FixupPass::isInvalidIdiv(const Assembly::IdivInstruction &idivInstr) {
    return dynamic_cast<const Assembly::ImmediateOperand *>(
               idivInstr.getOperand()) != nullptr;
}

bool FixupPass::isInvalidDiv(const Assembly::DivInstruction &divInstr) {
    return dynamic_cast<const Assembly::ImmediateOperand *>(
               divInstr.getOperand()) != nullptr;
}

bool FixupPass::isInvalidCmp(const Assembly::CmpInstruction &cmpInstr) {
    // If either both operands are memory-address operands or the second operand
    // is an immediate value, the `cmp` instruction is invalid.
    return (((dynamic_cast<const Assembly::StackOperand *>(
                  cmpInstr.getOperand1()) != nullptr) ||
             (dynamic_cast<const Assembly::DataOperand *>(
                  cmpInstr.getOperand1()) != nullptr)) &&
            ((dynamic_cast<const Assembly::StackOperand *>(
                  cmpInstr.getOperand2()) != nullptr) ||
             (dynamic_cast<const Assembly::DataOperand *>(
                  cmpInstr.getOperand2()) != nullptr))) ||
           ((dynamic_cast<const Assembly::ImmediateOperand *>(
                cmpInstr.getOperand2())) != nullptr);
}

bool FixupPass::isInvalidLargeImmediateCmp(
    const Assembly::CmpInstruction &cmpInstr) {
    // Check if it's a `Quadword` `Cmp` instruction with a large immediate
    // value.
    const auto *quadwordType =
        dynamic_cast<const Assembly::Quadword *>(cmpInstr.getType());
    if (quadwordType == nullptr) {
        return false;
    }

    const auto *immediateOp1 = dynamic_cast<const Assembly::ImmediateOperand *>(
        cmpInstr.getOperand1());
    const auto *immediateOp2 = dynamic_cast<const Assembly::ImmediateOperand *>(
        cmpInstr.getOperand2());

    const Assembly::ImmediateOperand *immediateOp = nullptr;
    if (immediateOp1 != nullptr) {
        immediateOp = immediateOp1;
    }
    else if (immediateOp2 != nullptr) {
        immediateOp = immediateOp2;
    }

    if (immediateOp == nullptr) {
        return false;
    }

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    const long value = static_cast<long>(immediateOp->getImmediate());
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

bool FixupPass::isInvalidLargeImmediatePush(
    const Assembly::PushInstruction &pushInstr) {
    const auto *immediateOp = dynamic_cast<const Assembly::ImmediateOperand *>(
        pushInstr.getOperand());
    if (immediateOp == nullptr) {
        return false;
    }

    // Check if immediate value is outside the range of a `Quadword` (32-bit
    // signed).
    const long value = static_cast<long>(immediateOp->getImmediate());
    return value > std::numeric_limits<int>::max() ||
           value < std::numeric_limits<int>::min();
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMov(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::MovInstruction &movInst) {
    auto newMov1 = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(movInst.getType()), cloneOperand(movInst.getSrc()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));
    auto newMov2 = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(movInst.getType()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()),
        cloneOperand(movInst.getDst()));

    *it = std::move(newMov1);
    it = instructions.insert(it + 1, std::move(newMov2));
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidMovsx(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::MovsxInstruction &movsxInst) {
    const auto *src = movsxInst.getSrc();
    const auto *dst = movsxInst.getDst();

    const bool invalidSrc =
        dynamic_cast<const Assembly::ImmediateOperand *>(src) != nullptr;
    const bool invalidDst =
        dynamic_cast<const Assembly::StackOperand *>(dst) != nullptr ||
        dynamic_cast<const Assembly::DataOperand *>(dst) != nullptr;

    if (invalidSrc && invalidDst) {
        auto newMov1 = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Longword>(), cloneOperand(src),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()));
        auto newMovsx = std::make_unique<Assembly::MovsxInstruction>(
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        auto newMov2 = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Quadword>(),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()),
            cloneOperand(dst));

        *it = std::move(newMov1);
        it = instructions.insert(it + 1, std::move(newMovsx));
        it = instructions.insert(it + 1, std::move(newMov2));
    }
    else if (invalidSrc) {
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Longword>(), cloneOperand(src),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()));
        auto newMovsx = std::make_unique<Assembly::MovsxInstruction>(
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()),
            cloneOperand(dst));

        *it = std::move(newMov);
        it = instructions.insert(it + 1, std::move(newMovsx));
    }
    else if (invalidDst) {
        auto newMovsx = std::make_unique<Assembly::MovsxInstruction>(
            cloneOperand(src), std::make_unique<Assembly::RegisterOperand>(
                                   std::make_unique<Assembly::R11>()));
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Quadword>(),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()),
            cloneOperand(dst));

        *it = std::move(newMovsx);
        it = instructions.insert(it + 1, std::move(newMov));
    }

    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidBinary(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::BinaryInstruction &binInstr) {
    if ((dynamic_cast<const Assembly::AddOperator *>(
             binInstr.getBinaryOperator()) != nullptr) ||
        (dynamic_cast<const Assembly::SubtractOperator *>(
             binInstr.getBinaryOperator()) != nullptr)) {
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(binInstr.getType()),
            cloneOperand(binInstr.getOperand1()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()));
        auto newBin = std::make_unique<Assembly::BinaryInstruction>(
            cloneBinaryOperator(binInstr.getBinaryOperator()),
            cloneAssemblyType(binInstr.getType()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()),
            cloneOperand(binInstr.getOperand2()));
        *it = std::move(newMov);
        it = instructions.insert(it + 1, std::move(newBin));
    }
    else if (dynamic_cast<const Assembly::MultiplyOperator *>(
                 binInstr.getBinaryOperator()) != nullptr) {
        auto newMov1 = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(binInstr.getType()),
            cloneOperand(binInstr.getOperand2()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        auto newImul = std::make_unique<Assembly::BinaryInstruction>(
            cloneBinaryOperator(binInstr.getBinaryOperator()),
            cloneAssemblyType(binInstr.getType()),
            cloneOperand(binInstr.getOperand1()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        auto newMov2 = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(binInstr.getType()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()),
            cloneOperand(binInstr.getOperand2()));
        *it = std::move(newMov1);
        it = instructions.insert(it + 1, std::move(newImul));
        it = instructions.insert(it + 1, std::move(newMov2));
    }

    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidIdiv(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::IdivInstruction &idivInstr) {
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(idivInstr.getType()),
        cloneOperand(idivInstr.getOperand()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));
    auto newIdiv = std::make_unique<Assembly::IdivInstruction>(
        cloneAssemblyType(idivInstr.getType()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));

    *it = std::move(newMov);
    it = instructions.insert(it + 1, std::move(newIdiv));
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidDiv(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::DivInstruction &divInstr) {
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(divInstr.getType()),
        cloneOperand(divInstr.getOperand()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));
    auto newDiv = std::make_unique<Assembly::DivInstruction>(
        cloneAssemblyType(divInstr.getType()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));

    *it = std::move(newMov);
    it = instructions.insert(it + 1, std::move(newDiv));
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteMovZeroExtend(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::MovZeroExtendInstruction &movZeroExtendInstr) {
    if (dynamic_cast<const Assembly::RegisterOperand *>(
            movZeroExtendInstr.getDst()) != nullptr) {
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Longword>(),
            cloneOperand(movZeroExtendInstr.getSrc()),
            cloneOperand(movZeroExtendInstr.getDst()));
        *it = std::move(newMov);
        return it;
    }
    else {
        auto movToR11 = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Longword>(),
            cloneOperand(movZeroExtendInstr.getSrc()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        auto movFromR11 = std::make_unique<Assembly::MovInstruction>(
            std::make_unique<Assembly::Quadword>(),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()),
            cloneOperand(movZeroExtendInstr.getDst()));

        *it = std::move(movToR11);
        it = instructions.insert(it + 1, std::move(movFromR11));
        return it;
    }
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidCmp(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::CmpInstruction &cmpInstr) {
    if (((dynamic_cast<const Assembly::StackOperand *>(
              cmpInstr.getOperand1()) != nullptr) ||
         (dynamic_cast<const Assembly::DataOperand *>(cmpInstr.getOperand1()) !=
          nullptr)) &&
        ((dynamic_cast<const Assembly::StackOperand *>(
              cmpInstr.getOperand2()) != nullptr) ||
         (dynamic_cast<const Assembly::DataOperand *>(cmpInstr.getOperand2()) !=
          nullptr))) {
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(cmpInstr.getType()),
            cloneOperand(cmpInstr.getOperand1()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()));
        auto newCmp = std::make_unique<Assembly::CmpInstruction>(
            cloneAssemblyType(cmpInstr.getType()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R10>()),
            cloneOperand(cmpInstr.getOperand2()));
        *it = std::move(newMov);
        it = instructions.insert(it + 1, std::move(newCmp));
    }
    else if (dynamic_cast<const Assembly::ImmediateOperand *>(
                 cmpInstr.getOperand2()) != nullptr) {
        auto newMov = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(cmpInstr.getType()),
            cloneOperand(cmpInstr.getOperand2()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        auto newCmp = std::make_unique<Assembly::CmpInstruction>(
            cloneAssemblyType(cmpInstr.getType()),
            cloneOperand(cmpInstr.getOperand1()),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        *it = std::move(newMov);
        it = instructions.insert(it + 1, std::move(newCmp));
    }

    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateMov(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::MovInstruction &movInst) {
    auto newMov1 = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(movInst.getType()), cloneOperand(movInst.getSrc()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));
    auto newMov2 = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(movInst.getType()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()),
        cloneOperand(movInst.getDst()));

    *it = std::move(newMov1);
    it = instructions.insert(it + 1, std::move(newMov2));
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLongwordImmediateMov(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions
    [[maybe_unused]],
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::MovInstruction &movInst) {
    const auto *immediateSrc =
        dynamic_cast<const Assembly::ImmediateOperand *>(movInst.getSrc());
    auto originalValue = immediateSrc->getImmediate();

    // Truncate the immediate value to 32 bits.
    auto truncatedValue =
        originalValue & std::numeric_limits<unsigned int>::max();

    auto newImmediate =
        std::make_unique<Assembly::ImmediateOperand>(truncatedValue);
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(movInst.getType()), std::move(newImmediate),
        cloneOperand(movInst.getDst()));

    *it = std::move(newMov);
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateBinary(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::BinaryInstruction &binInstr) {
    const Assembly::AssemblyType *binType = binInstr.getType();
    const Assembly::BinaryOperator *binOperator = binInstr.getBinaryOperator();
    const auto *immediateOp = dynamic_cast<const Assembly::ImmediateOperand *>(
        binInstr.getOperand1());
    const auto *otherOp = binInstr.getOperand2();
    bool isFirstOperand = true;
    if (immediateOp == nullptr) {
        immediateOp = dynamic_cast<const Assembly::ImmediateOperand *>(
            binInstr.getOperand2());
        otherOp = binInstr.getOperand1();
        isFirstOperand = false;
    }

    const bool otherIsMemory =
        (dynamic_cast<const Assembly::StackOperand *>(otherOp) != nullptr) ||
        (dynamic_cast<const Assembly::DataOperand *>(otherOp) != nullptr);
    auto otherOpCloneForLoad = otherIsMemory ? cloneOperand(otherOp) : nullptr;
    auto otherOpCloneForBin = cloneOperand(otherOp);
    auto otherOpCloneForStore = otherIsMemory ? cloneOperand(otherOp) : nullptr;
    auto immediateOpClone = cloneOperand(immediateOp);
    auto typeForMov = cloneAssemblyType(binType);
    auto typeForLoad = otherIsMemory ? cloneAssemblyType(binType) : nullptr;
    auto typeForBin = cloneAssemblyType(binType);
    auto typeForStore = otherIsMemory ? cloneAssemblyType(binType) : nullptr;
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        std::move(typeForMov), std::move(immediateOpClone),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));

    bool otherInRegister = false;
    std::unique_ptr<Assembly::Instruction> loadOtherMov;
    if (otherIsMemory) {
        loadOtherMov = std::make_unique<Assembly::MovInstruction>(
            std::move(typeForLoad), std::move(otherOpCloneForLoad),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        otherInRegister = true;
    }

    std::unique_ptr<Assembly::Operand> otherOperandForBin =
        otherInRegister ? std::make_unique<Assembly::RegisterOperand>(
                              std::make_unique<Assembly::R11>())
                        : std::move(otherOpCloneForBin);

    auto newBin = std::make_unique<Assembly::BinaryInstruction>(
        cloneBinaryOperator(binOperator), std::move(typeForBin),
        isFirstOperand ? std::make_unique<Assembly::RegisterOperand>(
                             std::make_unique<Assembly::R10>())
                       : std::move(otherOperandForBin),
        isFirstOperand ? std::move(otherOperandForBin)
                       : std::make_unique<Assembly::RegisterOperand>(
                             std::make_unique<Assembly::R10>()));

    *it = std::move(newMov);
    if (loadOtherMov) {
        it = instructions.insert(it + 1, std::move(loadOtherMov));
    }
    it = instructions.insert(it + 1, std::move(newBin));

    if (otherInRegister) {
        auto storeMov = std::make_unique<Assembly::MovInstruction>(
            std::move(typeForStore),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()),
            std::move(otherOpCloneForStore));
        it = instructions.insert(it + 1, std::move(storeMov));
    }

    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediateCmp(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::CmpInstruction &cmpInstr) {
    const Assembly::AssemblyType *cmpType = cmpInstr.getType();
    const auto *immediateOp = dynamic_cast<const Assembly::ImmediateOperand *>(
        cmpInstr.getOperand1());
    const auto *otherOp = cmpInstr.getOperand2();
    bool isFirstOperand = true;
    if (immediateOp == nullptr) {
        immediateOp = dynamic_cast<const Assembly::ImmediateOperand *>(
            cmpInstr.getOperand2());
        otherOp = cmpInstr.getOperand1();
        isFirstOperand = false;
    }

    auto immediateOpClone = cloneOperand(immediateOp);
    auto otherOpCloneForMov = cloneOperand(otherOp);
    auto otherOpCloneForCmp = cloneOperand(otherOp);
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        cloneAssemblyType(cmpType), std::move(immediateOpClone),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));

    const bool otherIsImmediate =
        dynamic_cast<const Assembly::ImmediateOperand *>(otherOp) != nullptr;
    bool otherInRegister = false;
    std::unique_ptr<Assembly::Instruction> otherMov;
    if (otherIsImmediate) {
        otherMov = std::make_unique<Assembly::MovInstruction>(
            cloneAssemblyType(cmpType), std::move(otherOpCloneForMov),
            std::make_unique<Assembly::RegisterOperand>(
                std::make_unique<Assembly::R11>()));
        otherInRegister = true;
    }

    std::unique_ptr<Assembly::Operand> otherForCmp =
        otherInRegister ? std::make_unique<Assembly::RegisterOperand>(
                              std::make_unique<Assembly::R11>())
                        : std::move(otherOpCloneForCmp);

    auto newCmp = std::make_unique<Assembly::CmpInstruction>(
        cloneAssemblyType(cmpType),
        isFirstOperand ? std::make_unique<Assembly::RegisterOperand>(
                             std::make_unique<Assembly::R10>())
                       : std::move(otherForCmp),
        isFirstOperand ? std::move(otherForCmp)
                       : std::make_unique<Assembly::RegisterOperand>(
                             std::make_unique<Assembly::R10>()));

    *it = std::move(newMov);
    if (otherMov) {
        it = instructions.insert(it + 1, std::move(otherMov));
    }
    it = instructions.insert(it + 1, std::move(newCmp));
    return it;
}

std::vector<std::unique_ptr<Assembly::Instruction>>::iterator
FixupPass::rewriteInvalidLargeImmediatePush(
    std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
    std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
    const Assembly::PushInstruction &pushInstr) {
    auto newMov = std::make_unique<Assembly::MovInstruction>(
        std::make_unique<Assembly::Quadword>(),
        cloneOperand(pushInstr.getOperand()),
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));
    auto newPush = std::make_unique<Assembly::PushInstruction>(
        std::make_unique<Assembly::RegisterOperand>(
            std::make_unique<Assembly::R10>()));

    *it = std::move(newMov);
    it = instructions.insert(it + 1, std::move(newPush));
    return it;
}
} // Namespace Assembly
