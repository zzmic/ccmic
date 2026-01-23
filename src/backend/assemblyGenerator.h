#ifndef BACKEND_ASSEMBLY_GENERATOR_H
#define BACKEND_ASSEMBLY_GENERATOR_H

#include "../frontend/frontendSymbolTable.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../midend/ir.h"
#include "assembly.h"
#include <unordered_map>

namespace Assembly {
/**
 * Class for generating assembly from the intermediate representation (IR)
 * program.
 */
class AssemblyGenerator {
  public:
    /**
     * Constructor for the `AssemblyGenerator` class.
     *
     * @param irStaticVariables The IR static variables.
     * @param frontendSymbolTable The frontend symbol table.
     */
    explicit AssemblyGenerator(
        const std::vector<std::unique_ptr<IR::StaticVariable>>
            &irStaticVariables,
        const AST::FrontendSymbolTable &frontendSymbolTable);

    /**
     * Generate assembly from the IR program.
     *
     * @param irProgram The IR program to generate assembly from.
     * @return The generated assembly.
     */
    [[nodiscard]] std::unique_ptr<Assembly::Program>
    generateAssembly(const IR::Program &irProgram);

  private:
    /**
     * The IR static variables.
     */
    const std::vector<std::unique_ptr<IR::StaticVariable>> &irStaticVariables;

    /**
     * The frontend symbol table.
     */
    const AST::FrontendSymbolTable &frontendSymbolTable;

    /**
     * Convert an IR function definition to assembly.
     */
    [[nodiscard]] std::unique_ptr<Assembly::FunctionDefinition>
    convertIRFunctionDefinitionToAssy(
        const IR::FunctionDefinition &irFunctionDefinition,
        std::unique_ptr<std::vector<std::unique_ptr<Assembly::Instruction>>>
            instructions);

    /**
     * Convert an IR static variable to assembly.
     *
     * @param irStaticVariable The IR static variable to convert.
     * @return The converted assembly static variable.
     */
    [[nodiscard]] std::unique_ptr<Assembly::StaticVariable>
    convertIRStaticVariableToAssy(const IR::StaticVariable &irStaticVariable);

    /**
     * Convert an IR instruction to assembly.
     *
     * @param irInstruction The IR instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRInstructionToAssy(
        const IR::Instruction &irInstruction,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR return instruction to assembly.
     *
     * @param returnInstr The IR return instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRReturnInstructionToAssy(
        const IR::ReturnInstruction &returnInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR unary instruction to assembly.
     *
     * @param unaryInstr The IR unary instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRUnaryInstructionToAssy(
        const IR::UnaryInstruction &unaryInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR binary instruction to assembly.
     *
     * @param binaryInstr The IR binary instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRBinaryInstructionToAssy(
        const IR::BinaryInstruction &binaryInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR label instruction to assembly.
     *
     * @param labelInstr The IR label instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRLabelInstructionToAssy(
        const IR::LabelInstruction &labelInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR jump instruction to assembly.
     *
     * @param jumpInstr The IR jump instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRJumpInstructionToAssy(
        const IR::JumpInstruction &jumpInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR jump-if-zero instruction to assembly.
     *
     * @param jumpIfZeroInstr The IR jump-if-zero instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRJumpIfZeroInstructionToAssy(
        const IR::JumpIfZeroInstruction &jumpIfZeroInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR jump-if-not-zero instruction to assembly.
     *
     * @param jumpIfNotZeroInstr The IR jump-if-not-zero instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRJumpIfNotZeroInstructionToAssy(
        const IR::JumpIfNotZeroInstruction &jumpIfNotZeroInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR copy instruction to assembly.
     *
     * @param copyInstr The IR copy instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRCopyInstructionToAssy(
        const IR::CopyInstruction &copyInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR sign-extend instruction to assembly.
     *
     * @param signExtendInstr The IR sign-extend instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRSignExtendInstructionToAssy(
        const IR::SignExtendInstruction &signExtendInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR truncate instruction to assembly.
     *
     * @param truncateInstr The IR truncate instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRTruncateInstructionToAssy(
        const IR::TruncateInstruction &truncateInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR function-call instruction to assembly.
     *
     * @param functionCallInstr The IR function-call instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRFunctionCallInstructionToAssy(
        const IR::FunctionCallInstruction &functionCallInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR zero-extend instruction to assembly.
     *
     * @param zeroExtendInstr The IR zero-extend instruction to convert.
     * @param instructions The assembly instructions.
     */
    void convertIRZeroExtendInstructionToAssy(
        const IR::ZeroExtendInstruction &zeroExtendInstr,
        std::vector<std::unique_ptr<Assembly::Instruction>> &instructions);

    /**
     * Convert an IR value to an assembly operand.
     *
     * @param value The IR value to convert.
     * @return The converted assembly operand.
     */
    [[nodiscard]] std::unique_ptr<Assembly::Operand>
    convertValue(const IR::Value *value);

    /**
     * Determine the assembly type of an IR value.
     *
     * @param value The IR value to determine the assembly type of.
     * @return The assembly type of the IR value.
     */
    [[nodiscard]] std::unique_ptr<Assembly::AssemblyType>
    determineAssemblyType(const IR::Value *value);

    /**
     * Determine the assembly type of a mov instruction.
     *
     * @param src The source operand of the move instruction.
     * @param dst The destination operand of the move instruction.
     * @param irSrc The IR source value of the move instruction.
     * @param irDst The IR destination value of the move instruction.
     * @return The assembly type of the move instruction.
     */
    [[nodiscard]] std::unique_ptr<Assembly::AssemblyType>
    determineMovType(const Assembly::Operand *src, const Assembly::Operand *dst,
                     const IR::Value *irSrc, const IR::Value *irDst);

  public:
    /**
     * Convert an AST type to an assembly type.
     *
     * @param astType The AST type to convert.
     * @return The converted assembly type.
     */
    [[nodiscard]] static std::unique_ptr<Assembly::AssemblyType>
    convertASTTypeToAssemblyType(const AST::Type *astType);
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_GENERATOR_H
