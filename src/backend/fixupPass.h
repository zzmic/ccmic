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
    bool isInvalidBinary(std::shared_ptr<Assembly::BinaryInstruction> binInstr);
    bool isInvalidIdiv(std::shared_ptr<Assembly::IdivInstruction> idivInstr);
    bool isInvalidCmp(std::shared_ptr<Assembly::CmpInstruction> cmpInstr);
    // `std::vector<std::shared_ptr<Assembly::Instruction>>::iterator` is an
    // iterator to a vector of shared pointers to `Instruction` objects.
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidBinary(
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
};
} // Namespace Assembly

#endif // BACKEND_FIXUP_PASS_H
