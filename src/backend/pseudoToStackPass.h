#ifndef BACKEND_PSEUDO_TO_STACK_PASS_H
#define BACKEND_PSEUDO_TO_STACK_PASS_H

#include "assembly.h"
#include "backendSymbolTable.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Assembly {
/**
 * Class for performing pseudo-to-stack pass on the assembly program.
 */
class PseudoToStackPass {
  public:
    /**
     * Replace pseudo registers with stack operands and associate the stack size
     * with each function.
     *
     * @param topLevels The top-levels of the assembly program.
     * @param backendSymbolTable The backend symbol table.
     */
    void replacePseudoWithStackAndAssociateStackSize(
        std::vector<std::unique_ptr<TopLevel>> &topLevels,
        const BackendSymbolTable &backendSymbolTable);

  private:
    /**
     * Replace pseudo registers with stack operands in an instruction.
     *
     * @param instruction The instruction to replace pseudo registers with stack
     * operands.
     * @param backendSymbolTable The backend symbol table.
     */
    void
    replacePseudoWithStack(std::unique_ptr<Assembly::Instruction> &instruction,
                           const BackendSymbolTable &backendSymbolTable);

    /**
     * Replace an operand with a stack operand.
     *
     * @param operand The operand to replace.
     * @param backendSymbolTable The backend symbol table.
     * @return The replaced operand.
     */
    [[nodiscard]] std::unique_ptr<Assembly::Operand>
    replaceOperand(const Assembly::Operand *operand,
                   const BackendSymbolTable &backendSymbolTable);

    /**
     * Check if all pseudo registers in a function definition have been replaced
     * with stack operands.
     *
     * @param functionDefinition The function definition to check.
     */
    void checkPseudoRegistersInFunctionDefinitionReplaced(
        const Assembly::FunctionDefinition &functionDefinition);

    /**
     * A map from pseudo registers to stack offsets.
     */
    std::unordered_map<std::string, int> pseudoToStackMap;

    /**
     * The current stack offset.
     */
    int offset = -4;
};
} // namespace Assembly

#endif // BACKEND_PSEUDO_TO_STACK_PASS_H
