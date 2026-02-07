#ifndef BACKEND_FIXUP_PASS_H
#define BACKEND_FIXUP_PASS_H

#include "assembly.h"
#include <memory>
#include <vector>

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
    static void fixup(std::vector<std::unique_ptr<TopLevel>> &topLevels);

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
    static void insertAllocateStackInstruction(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        int stackSize);

    /**
     * Rewrite a function definition.
     *
     * Rewrite: `FunctionDefinition(instructions)` ->
     * `FunctionDefinition(instructions)` + `AllocateStack(stackSize)`.
     *
     * @param functionDefinition The function definition to rewrite.
     */
    static void
    rewriteFunctionDefinition(FunctionDefinition &functionDefinition);

    /**
     * Check if a mov instruction is invalid.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidMov(const Assembly::MovInstruction &movInstr);

    /**
     * Check if a mov instruction has a large immediate value.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] static bool
    isInvalidLargeImmediateMov(const Assembly::MovInstruction &movInstr);

    /**
     * Check if a mov instruction is an invalid longword immediate mov
     * instruction.
     *
     * @param movInstr The mov instruction to check.
     * @return True if the mov instruction is an invalid longword immediate mov
     * instruction, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidLongwordImmediateMov(const Assembly::MovInstruction &movInstr);

    /**
     * Check if a movsx instruction is invalid.
     *
     * @param movsxInstr The movsx instruction to check.
     * @return True if the movsx instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidMovsx(const Assembly::MovsxInstruction &movsxInstr);

    /**
     * Check if a binary instruction is invalid.
     *
     * @param binInstr The binary instruction to check.
     * @return True if the binary instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidBinary(const Assembly::BinaryInstruction &binInstr);

    /**
     * Check if a binary instruction has a large immediate value.
     *
     * @param binInstr The binary instruction to check.
     * @return True if the binary instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] static bool
    isInvalidLargeImmediateBinary(const Assembly::BinaryInstruction &binInstr);

    /**
     * Check if an idiv instruction is invalid.
     *
     * @param idivInstr The idiv instruction to check.
     * @return True if the idiv instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidIdiv(const Assembly::IdivInstruction &idivInstr);

    /**
     * Check if a div instruction is invalid.
     *
     * @param divInstr The div instruction to check.
     * @return True if the div instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidDiv(const Assembly::DivInstruction &divInstr);

    /**
     * Check if a cmp instruction is invalid.
     *
     * @param cmpInstr The cmp instruction to check.
     * @return True if the cmp instruction is invalid, false otherwise.
     */
    [[nodiscard]] static bool
    isInvalidCmp(const Assembly::CmpInstruction &cmpInstr);

    /**
     * Check if a cmp instruction has a large immediate value.
     *
     * @param cmpInstr The cmp instruction to check.
     * @return True if the cmp instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] static bool
    isInvalidLargeImmediateCmp(const Assembly::CmpInstruction &cmpInstr);

    /**
     * Check if a push instruction has a large immediate value.
     *
     * @param pushInstr The push instruction to check.
     * @return True if the push instruction has a large immediate value, false
     * otherwise.
     */
    [[nodiscard]] static bool
    isInvalidLargeImmediatePush(const Assembly::PushInstruction &pushInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMov(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::MovInstruction &movInst);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateMov(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::MovInstruction &movInst);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLongwordImmediateMov(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::MovInstruction &movInst);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidMovsx(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::MovsxInstruction &movsxInst);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidBinary(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::BinaryInstruction &binInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateBinary(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::BinaryInstruction &binInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidIdiv(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::IdivInstruction &idivInstr);

    /**
     * Rewrite an invalid div instruction.
     *
     * Rewrite: `Div(type, Imm(val))` ->
     * `Mov(type, Imm(val), Reg(R10))` + `Div(type, Reg(R10))`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the div instruction.
     * @param divInstr The div instruction to rewrite.
     * @return The iterator to the new div instruction.
     */
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidDiv(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::DivInstruction &divInstr);

    /**
     * Rewrite a `MovZeroExtend` instruction.
     *
     * If the destination is a register:
     * Rewrite: `MovZeroExtend(src, Reg)` -> `Mov(Longword, src, Reg)`.
     *
     * If the destination is in memory:
     * Rewrite: `MovZeroExtend(src, Stack/Data)` ->
     * `Mov(Longword, src, Reg(R11))` + `Mov(Quadword, Reg(R11), Stack/Data)`.
     *
     * @param instructions The instructions of the function.
     * @param it The iterator to the `MovZeroExtend` instruction.
     * @param movZeroExtendInstr The `MovZeroExtend` instruction to rewrite.
     * @return The iterator to the last new instruction.
     */
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteMovZeroExtend(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::MovZeroExtendInstruction &movZeroExtendInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidCmp(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::CmpInstruction &cmpInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediateCmp(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::CmpInstruction &cmpInstr);

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
    [[nodiscard]] static std::vector<
        std::unique_ptr<Assembly::Instruction>>::iterator
    rewriteInvalidLargeImmediatePush(
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions,
        std::vector<std::unique_ptr<Assembly::Instruction>>::iterator it,
        const Assembly::PushInstruction &pushInstr);
};
} // Namespace Assembly

#endif // BACKEND_FIXUP_PASS_H
