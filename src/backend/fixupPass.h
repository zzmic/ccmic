#ifndef BACKEND_FIXUP_PASS_H
#define BACKEND_FIXUP_PASS_H

#include "assembly.h"
#include <memory>

namespace Assembly {
/**
 * Class for performing fixup passes on the assembly program.
 */
class FixupPass {
  public:
    /**
     * Perform fixup passes on the assembly program.
     *
     * @param topLevels The top-levels of the assembly program.
     */
    void
    fixup(std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels);

  private:
    /**
     * Insert an allocate-stack instruction at the beginning of a function.
     *
     * Rewrite: `FunctionDefinition(instructions)` ->
     * `FunctionDefinition(instructions)` + `AllocateStack(stackSize)`.
     *
     * @param instructions The instructions of the function.
     * @param stackSize The stack size of the function.
     */
    void insertAllocateStackInstruction(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        int stackSize);

    /**
     * Rewrite a function definition.
     *
     * Rewrite: `FunctionDefinition(instructions)` ->
     * `FunctionDefinition(instructions)` + `AllocateStack(stackSize)`.
     *
     * @param functionDefinition The function definition to rewrite.
     */
    void rewriteFunctionDefinition(
        std::shared_ptr<FunctionDefinition> functionDefinition);

    /**
     * Check if a mov instruction is invalid.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction is invalid, false otherwise.
     */
    [[nodiscard]] bool
    isInvalidMov(std::shared_ptr<Assembly::MovInstruction> movInstr);

    /**
     * Check if a mov instruction has a large immediate value.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] bool isInvalidLargeImmediateMov(
        std::shared_ptr<Assembly::MovInstruction> movInstr);

    /**
     * Check if a mov instruction is an invalid longword immediate mov
     * instruction.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction is an invalid longword immediate mov
     * instruction, false otherwise.
     */
    [[nodiscard]] bool isInvalidLongwordImmediateMov(
        std::shared_ptr<Assembly::MovInstruction> movInstr);

    /**
     * Check if a movsx instruction is invalid.
     *
     * @param movsxInstr The movsx instruction to check.
     * @return True if the movsx instruction is invalid, false otherwise.
     */
    [[nodiscard]] bool
    isInvalidMovsx(std::shared_ptr<Assembly::MovsxInstruction> movsxInstr);

    /**
     * Check if a binary instruction is invalid.
     *
     * @param binInstr The binary instruction to check.
     * @return True if the binary instruction is invalid, false otherwise.
     */
    [[nodiscard]] bool
    isInvalidBinary(std::shared_ptr<Assembly::BinaryInstruction> binInstr);

    /**
     * Check if a binary instruction has a large immediate value.
     *
     * @param binInstr The binary instruction to check.
     * @return True if the binary instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] bool isInvalidLargeImmediateBinary(
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);

    /**
     * Check if an idiv instruction is invalid.
     *
     * @param idivInstr The idiv instruction to check.
     * @return True if the idiv instruction is invalid, false otherwise.
     */
    [[nodiscard]] bool
    isInvalidIdiv(std::shared_ptr<Assembly::IdivInstruction> idivInstr);

    /**
     * Check if a cmp instruction is invalid.
     *
     * @param cmpInstr The cmp instruction to check.
     * @return True if the cmp instruction is invalid, false otherwise.
     */
    [[nodiscard]] bool
    isInvalidCmp(std::shared_ptr<Assembly::CmpInstruction> cmpInstr);

    /**
     * Check if a cmp instruction has a large immediate value.
     *
     * @param cmpInstr The cmp instruction to check.
     * @return True if the cmp instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] bool isInvalidLargeImmediateCmp(
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);

    /**
     * Check if a push instruction has a large immediate value.
     *
     * @param pushInstr The push instruction to check.
     * @return True if the push instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] bool isInvalidLargeImmediatePush(
        std::shared_ptr<Assembly::PushInstruction> pushInstr);

    /**
     * Rewrite an invalid mov instruction.
     *
     * Rewrite: `Mov(Stack/Data, Stack/Data)` ->
     * `Mov(Quadword, Stack/Data, Reg(R10))` + `Mov(Quadword, Reg(R10),
     * Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the mov instruction.
     * @param movInst The mov instruction to rewrite.
     * @return The iterator to the new mov instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);

    /**
     * Rewrite an invalid large immediate mov instruction.
     *
     * Rewrite: `Mov(Quadword, Imm(large), Stack/Data)` ->
     * `Mov(Quadword, Imm(large), Reg(R10))` + `Mov(Quadword, Reg(R10),
     * Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the mov instruction.
     * @param movInst The mov instruction to rewrite.
     * @return The iterator to the new mov instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);

    /**
     * Rewrite an invalid longword immediate mov instruction.
     *
     * Rewrite: `Mov(Longword, Imm(large), Reg)` ->
     * `Mov(Longword, Imm(truncated), Reg)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the mov instruction.
     * @param movInst The mov instruction to rewrite.
     * @return The iterator to the new mov instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLongwordImmediateMov(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovInstruction> movInst);

    /**
     * Rewrite an invalid movsx instruction.
     *
     * Rewrite: `Movsx(Imm(large), Stack/Data)` ->
     * `Mov(Longword, Imm(large), Reg(R10))` + `Movsx(Reg(R10), Reg(R11))` +
     * `Mov(Quadword, Reg(R11), Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the movsx instruction.
     * @param movsxInst The movsx instruction to rewrite.
     * @return The iterator to the new movsx instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMovsx(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::MovsxInstruction> movsxInst);

    /**
     * Rewrite an invalid binary instruction.
     *
     * Rewrite: `Binary(op, Stack/Data, Stack/Data)` ->
     * `Mov(Quadword, Stack/Data, Reg(R10))` + `Binary(op, Quadword, Reg(R10),
     * Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the binary instruction.
     * @param binInstr The binary instruction to rewrite.
     * @return The iterator to the new binary instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidBinary(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);

    /**
     * Rewrite an invalid large immediate binary instruction.
     *
     * Rewrite: `Binary(op, Quadword, Imm(large), Reg)` ->
     * `Mov(Quadword, Imm(large), Reg(R10))` + `Binary(op, Quadword, Reg(R10),
     *  Reg)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the binary instruction.
     * @param binInstr The binary instruction to rewrite.
     * @return The iterator to the new binary instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateBinary(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::BinaryInstruction> binInstr);

    /**
     * Rewrite an invalid idiv instruction.
     *
     * Rewrite: `Idiv(Quadword, Imm(large))` ->
     * `Mov(Quadword, Imm(large), Reg(R10))` + `Idiv(Quadword, Reg(R10))`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the idiv instruction.
     * @param idivInst The idiv instruction to rewrite.
     * @return The iterator to the new idiv instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidIdiv(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::IdivInstruction> idivInstr);

    /**
     * Rewrite an invalid cmp instruction.
     *
     * Rewrite: `Cmp(Stack/Data, Stack/Data)` ->
     * `Mov(Quadword, Stack/Data, Reg(R10))` + `Cmp(Quadword, Reg(R10),
     * Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the cmp instruction.
     * @param cmpInst The cmp instruction to rewrite.
     * @return The iterator to the new cmp instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidCmp(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);

    /**
     * Rewrite an invalid large immediate cmp instruction.
     *
     * Rewrite: `Cmp(Quadword, Imm(large), Reg)` ->
     * `Mov(Quadword, Imm(large), Reg(R10))` + `Cmp(Quadword, Reg(R10), Reg)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the cmp instruction.
     * @param cmpInst The cmp instruction to rewrite.
     * @return The iterator to the new cmp instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateCmp(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::CmpInstruction> cmpInstr);

    /**
     * Rewrite an invalid large immediate push instruction.
     *
     * Rewrite: `Push(Imm(large))` -> `Mov(Quadword, Imm(large), Reg(R10))` +
     * `Push(Reg(R10))`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the push instruction.
     * @param pushInst The push instruction to rewrite.
     * @return The iterator to the new push instruction.
     */
    [[nodiscard]] std::vector<std::shared_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediatePush(
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions,
        std::vector<std::shared_ptr<Assembly::Instruction>>::iterator it,
        std::shared_ptr<Assembly::PushInstruction> pushInstr);
};
} // Namespace Assembly

#endif // BACKEND_FIXUP_PASS_H
