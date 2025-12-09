#ifndef UTILS_PRETTY_PRINTERS_H
#define UTILS_PRETTY_PRINTERS_H

#include "../backend/assembly.h"
#include "../midend/ir.h"
#include <iostream>

/**
 * Class containing static pretty-printer methods for IR and assembly programs.
 */
class PrettyPrinters {
  public:
    /**
     * Print the IR program to stdout.
     *
     * @param irProgram The IR program to print.
     * @param irStaticVariables The list of IR static variables.
     */
    static void printIRProgram(
        const std::shared_ptr<IR::Program> &irProgram,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
            &irStaticVariables);

    /**
     * Print the assembly program to stdout.
     *
     * @param assemblyProgram The assembly program to print.
     */
    static void printAssemblyProgram(
        const std::shared_ptr<Assembly::Program> &assemblyProgram);

  private:
    /**
     * Print an IR function definition to stdout.
     *
     * @param functionDefinition The IR function definition to print.
     */
    static void printIRFunctionDefinition(
        const std::shared_ptr<IR::FunctionDefinition> &functionDefinition);

    /**
     * Print an IR static variable to stdout.
     *
     * @param staticVariable The IR static variable to print.
     */
    static void printIRStaticVariable(
        const std::shared_ptr<IR::StaticVariable> &staticVariable);

    /**
     * Print an IR instruction to stdout.
     *
     * @param instruction The IR instruction to print.
     */
    static void
    printIRInstruction(const std::shared_ptr<IR::Instruction> &instruction);

    /**
     * Print an IR return instruction to stdout.
     *
     * @param returnInstruction The IR return instruction to print.
     */
    static void printIRReturnInstruction(
        const std::shared_ptr<IR::ReturnInstruction> &returnInstruction);

    /**
     * Print an IR sign-extend instruction to stdout.
     *
     * @param signExtendInstruction The IR sign-extend instruction to print.
     */
    static void printIRSignExtendInstruction(
        const std::shared_ptr<IR::SignExtendInstruction>
            &signExtendInstruction);

    /**
     * Print an IR truncate instruction to stdout.
     *
     * @param truncateInstruction The IR truncate instruction to print.
     */
    static void printIRTruncateInstruction(
        const std::shared_ptr<IR::TruncateInstruction> &truncateInstruction);

    /**
     * Print an IR unary instruction to stdout.
     *
     * @param unaryInstruction The IR unary instruction to print.
     */
    static void printIRUnaryInstruction(
        const std::shared_ptr<IR::UnaryInstruction> &unaryInstruction);

    /**
     * Print an IR binary instruction to stdout.
     *
     * @param binaryInstruction The IR binary instruction to print.
     */
    static void printIRBinaryInstruction(
        const std::shared_ptr<IR::BinaryInstruction> &binaryInstruction);

    /**
     * Print an IR copy instruction to stdout.
     *
     * @param copyInstruction The IR copy instruction to print.
     */
    static void printIRCopyInstruction(
        const std::shared_ptr<IR::CopyInstruction> &copyInstruction);

    /**
     * Print an IR jump instruction to stdout.
     *
     * @param jumpInstruction The IR jump instruction to print.
     */
    static void printIRJumpInstruction(
        const std::shared_ptr<IR::JumpInstruction> &jumpInstruction);

    /**
     * Print an IR jump-if-zero instruction to stdout.
     *
     * @param jumpIfZeroInstruction The IR jump-if-zero instruction to print.
     */
    static void printIRJumpIfZeroInstruction(
        const std::shared_ptr<IR::JumpIfZeroInstruction>
            &jumpIfZeroInstruction);

    /**
     * Print an IR jump-if-not-zero instruction to stdout.
     *
     * @param jumpIfNotZeroInstruction The IR jump-if-not-zero instruction to
     * print.
     */
    static void printIRJumpIfNotZeroInstruction(
        const std::shared_ptr<IR::JumpIfNotZeroInstruction>
            &jumpIfNotZeroInstruction);

    /**
     * Print an IR label instruction to stdout.
     *
     * @param labelInstruction The IR label instruction to print.
     */
    static void printIRLabelInstruction(
        const std::shared_ptr<IR::LabelInstruction> &labelInstruction);

    /**
     * Print an IR function call instruction to stdout.
     *
     * @param functionCallInstruction The IR function call instruction to print.
     */
    static void printIRFunctionCallInstruction(
        const std::shared_ptr<IR::FunctionCallInstruction>
            &functionCallInstruction);

    /**
     * Print an assembly function definition to stdout.
     *
     * @param functionDefinition The assembly function definition to print.
     */
    static void printAssyFunctionDefinition(
        const std::shared_ptr<Assembly::FunctionDefinition>
            &functionDefinition);

    /**
     * Print an assembly static variable to stdout.
     *
     * @param staticVariable The assembly static variable to print.
     */
    static void printAssyStaticVariable(
        const std::shared_ptr<Assembly::StaticVariable> &staticVariable);

    /**
     * Print an assembly instruction to stdout.
     *
     * @param instruction The assembly instruction to print.
     */
    static void printAssyInstruction(
        const std::shared_ptr<Assembly::Instruction> &instruction);

    /**
     * Print an assembly move instruction to stdout.
     *
     * @param movInstruction The assembly move instruction to print.
     */
    static void printAssyMovInstruction(
        const std::shared_ptr<Assembly::MovInstruction> &movInstruction);

    /**
     * Print an assembly move-with-sign-extend instruction to stdout.
     *
     * @param movsxInstruction The assembly move-with-sign-extend instruction to
     * print.
     */
    static void printAssyMovsxInstruction(
        const std::shared_ptr<Assembly::MovsxInstruction> &movsxInstruction);

    /**
     * Print an assembly return instruction to stdout.
     *
     * @param retInstruction The assembly return instruction to print.
     */
    static void printAssyRetInstruction(
        const std::shared_ptr<Assembly::RetInstruction> &retInstruction);

    /**
     * Print an assembly push instruction to stdout.
     *
     * @param pushInstruction The assembly push instruction to print.
     */
    static void printAssyPushInstruction(
        const std::shared_ptr<Assembly::PushInstruction> &pushInstruction);

    /**
     * Print an assembly call instruction to stdout.
     *
     * @param callInstruction The assembly call instruction to print.
     */
    static void printAssyCallInstruction(
        const std::shared_ptr<Assembly::CallInstruction> &callInstruction);

    /**
     * Print an assembly unary instruction to stdout.
     *
     * @param unaryInstruction The assembly unary instruction to print.
     */
    static void printAssyUnaryInstruction(
        const std::shared_ptr<Assembly::UnaryInstruction> &unaryInstruction);

    /**
     * Print an assembly binary instruction to stdout.
     *
     * @param binaryInstruction The assembly binary instruction to print.
     */
    static void printAssyBinaryInstruction(
        const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction);

    /**
     * Print an assembly compare instruction to stdout.
     *
     * @param cmpInstruction The assembly compare instruction to print.
     */
    static void printAssyCmpInstruction(
        const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction);

    /**
     * Print an assembly signed-integer-division instruction to stdout.
     *
     * @param idivInstruction The assembly signed-integer-division instruction
     * to print.
     */
    static void printAssyIdivInstruction(
        const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction);

    /**
     * Print an assembly covert-doubleword-to-quadword instruction to stdout.
     *
     * @param cdqInstruction The assembly covert-doubleword-to-quadword
     * instruction to print.
     */
    static void printAssyCdqInstruction(
        const std::shared_ptr<Assembly::CdqInstruction> &cdqInstruction);

    /**
     * Print an assembly jump instruction to stdout.
     *
     * @param jmpInstruction The assembly jump instruction to print.
     */
    static void printAssyJmpInstruction(
        const std::shared_ptr<Assembly::JmpInstruction> &jmpInstruction);

    /**
     * Print an assembly conditional-jump instruction to stdout.
     *
     * @param jmpCCInstruction The assembly conditional-jump instruction to
     * print.
     */
    static void printAssyJmpCCInstruction(
        const std::shared_ptr<Assembly::JmpCCInstruction> &jmpCCInstruction);

    /**
     * Print an assembly set-on-condition instruction to stdout.
     *
     * @param setCCInstruction The assembly set-on-condition instruction to
     * print.
     */
    static void printAssySetCCInstruction(
        const std::shared_ptr<Assembly::SetCCInstruction> &setCCInstruction);

    /**
     * Print an assembly label instruction to stdout.
     *
     * @param labelInstruction The assembly label instruction to print.
     */
    static void printAssyLabelInstruction(
        const std::shared_ptr<Assembly::LabelInstruction> &labelInstruction);

    /**
     * Prepend an underscore to the identifier if the underlying OS is macOS.
     *
     * @param identifier The identifier to potentially modify.
     */
    static void prependUnderscoreToIdentifierIfMacOS(
        [[maybe_unused]] std::string &identifier);
};

#endif // UTILS_PRETTY_PRINTERS_H
