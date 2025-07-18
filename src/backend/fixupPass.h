#ifndef BACKEND_FIXUP_PASS_H
#define BACKEND_FIXUP_PASS_H

#include "assembly.h"
#include <memory>

namespace Assembly {
class FixupPass {
  public:
    void
    fixup(std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels);

  private:
    void insertAllocateStackInstruction(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        int stackSize);
    void rewriteFunctionDefinition(
        std::shared_ptr<FunctionDefinition> functionDefinition);
    bool isInvalidMov(std::shared_ptr<Assembly::MovInstruction> movInstr);
    bool isInvalidLargeImmediateMov(
        std::shared_ptr<Assembly::MovInstruction> movInstr);
    bool isInvalidLongwordImmediateMov(
        std::shared_ptr<Assembly::MovInstruction> movInstr);
    bool isInvalidMovsx(std::shared_ptr<Assembly::MovsxInstruction> movsxInstr);
    bool isInvalidBinary(std::shared_ptr<Assembly::BinaryInstruction> binInstr);
    bool isInvalidLargeImmediateBinary(
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);
    bool isInvalidIdiv(std::shared_ptr<Assembly::IdivInstruction> idivInstr);
    bool isInvalidCmp(std::shared_ptr<Assembly::CmpInstruction> cmpInstr);
    bool isInvalidLargeImmediateCmp(
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);
    bool isInvalidLargeImmediatePush(
        std::shared_ptr<Assembly::PushInstruction> pushInstr);
    // `std::vector<std::shared_ptr<Assembly::Instruction>>::iterator` is an
    // iterator to a vector of shared pointers to `Instruction` objects.
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLongwordImmediateMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMovsx(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovsxInstruction> movsxInst);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidBinary(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateBinary(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidIdiv(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::IdivInstruction> idivInstr);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidCmp(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateCmp(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediatePush(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::PushInstruction> pushInstr);
};
} // Namespace Assembly

#endif // BACKEND_FIXUP_PASS_H
