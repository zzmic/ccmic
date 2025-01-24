#ifndef BACKEND_PSEUDO_TO_STACK_PASS_H
#define BACKEND_PSEUDO_TO_STACK_PASS_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include "assembly.h"
#include <unordered_map>

namespace Assembly {
class PseudoToStackPass {
  public:
    PseudoToStackPass(
        std::unordered_map<std::string,
                           std::pair<std::shared_ptr<Type>,
                                     std::shared_ptr<AST::IdentifierAttribute>>>
            symbols);
    void replacePseudoWithStackAndAssociateStackSize(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &topLevels);

  private:
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        symbols;
    void replacePseudoWithStack(
        std::shared_ptr<Assembly::Instruction> &instruction,
        std::unordered_map<std::string, int> &pseudoToStackMap,
        int &currentOffset);
    void replaceOperand(std::shared_ptr<Assembly::Operand> &operand,
                        std::unordered_map<std::string, int> &pseudoToStackMap,
                        int &currentOffset);
    void checkPseudoRegistersInFunctionDefinitionReplaced(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition);
};
} // namespace Assembly

#endif // BACKEND_PSEUDO_TO_STACK_PASS_H
