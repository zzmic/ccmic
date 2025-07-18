#ifndef BACKEND_PSEUDO_TO_STACK_PASS_H
#define BACKEND_PSEUDO_TO_STACK_PASS_H

#include "assembly.h"
#include "backendSymbolTable.h"
#include <unordered_map>

namespace Assembly {
class PseudoToStackPass {
  public:
    explicit PseudoToStackPass() = default;
    void replacePseudoWithStackAndAssociateStackSize(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &topLevels);

  private:
    void
    replacePseudoWithStack(std::shared_ptr<Assembly::Instruction> &instruction);
    std::shared_ptr<Assembly::Operand>
    replaceOperand(std::shared_ptr<Assembly::Operand> operand);
    void checkPseudoRegistersInFunctionDefinitionReplaced(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition);
    std::unordered_map<std::string, int> pseudoToStackMap;
    int offset = -4;
};
} // namespace Assembly

#endif // BACKEND_PSEUDO_TO_STACK_PASS_H
