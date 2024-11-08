

#ifndef FIXUP_PASS_H
#define FIXUP_PASS_H

#include "assembly.h"
#include <memory>

namespace Assembly {
class FixupPass {
  public:
    void fixup(std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
                   function,
               int stackSize);

  private:
    void insertAllocateStack(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        int stackSize);
    bool isInvalidMov(std::shared_ptr<Assembly::MovInstruction> movInstr);
    // `std::vector<std::shared_ptr<Assembly::Instruction>>::iterator` is an
    // iterator to a vector of shared pointers to `Instruction` objects.
    std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);
};
} // Namespace Assembly

#endif // FIXUP_PASS_H
