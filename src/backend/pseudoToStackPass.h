#ifndef BACKEND_PSEUDO_TO_STACK_PASS_H
#define BACKEND_PSEUDO_TO_STACK_PASS_H

#include "assembly.h"
#include "backendSymbolTable.h"
#include <unordered_map>

namespace Assembly {
/**
 * Class for performing pseudo-to-stack pass on the assembly program.
 */
class PseudoToStackPass {
  public:
    /**
     * Default constructor for the pseudo-to-stack pass class.
     */
    explicit PseudoToStackPass() = default;

    /**
     * Replace pseudo registers with stack operands and associate the stack size
     * with each function.
     *
     * @param topLevels The top-levels of the assembly program.
     * @param backendSymbolTable The backend symbol table.
     */
    void replacePseudoWithStackAndAssociateStackSize(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &topLevels,
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
    replacePseudoWithStack(std::shared_ptr<Assembly::Instruction> &instruction,
                           const BackendSymbolTable &backendSymbolTable);

    /**
     * Replace an operand with a stack operand.
     *
     * @param operand The operand to replace.
     * @param backendSymbolTable The backend symbol table.
     * @return The replaced operand.
     */
    [[nodiscard]] std::shared_ptr<Assembly::Operand>
    replaceOperand(std::shared_ptr<Assembly::Operand> operand,
                   const BackendSymbolTable &backendSymbolTable);

    /**
     * Check if all pseudo registers in a function definition have been replaced
     * with stack operands.
     *
     * @param functionDefinition The function definition to check.
     */
    void checkPseudoRegistersInFunctionDefinitionReplaced(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition);

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
