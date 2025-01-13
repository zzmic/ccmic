#ifndef BACKEND_PSEUDO_TO_STACK_PASS_H
#define BACKEND_PSEUDO_TO_STACK_PASS_H

#include "assembly.h"
#include <unordered_map>

namespace Assembly {
class PseudoToStackPass {
  public:
    void replacePseudoWithStackAndAssociateStackSize(
        std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
            &functionDefinitions);

  private:
    void replacePseudoWithStack(
        std::shared_ptr<Assembly::Instruction> &instruction,
        std::unordered_map<std::string, int> &pseudoToStackMap,
        int &currentOffset);
    void replaceOperand(std::shared_ptr<Assembly::Operand> &operand,
                        std::unordered_map<std::string, int> &pseudoToStackMap,
                        int &currentOffset);
};
} // namespace Assembly

#endif // BACKEND_PSEUDO_TO_STACK_PASS_H
