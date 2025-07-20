#ifndef BACKEND_ASSEMBLY_GENERATOR_H
#define BACKEND_ASSEMBLY_GENERATOR_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include "../midend/ir.h"
#include "assembly.h"
#include <unordered_map>

namespace Assembly {
class AssemblyGenerator {
  public:
    explicit AssemblyGenerator(
        std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
            irStaticVariables);
    std::shared_ptr<Assembly::Program>
    generateAssembly(std::shared_ptr<IR::Program> irProgram);

  private:
    std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        irStaticVariables;
    std::shared_ptr<Assembly::FunctionDefinition>
    convertIRFunctionDefinitionToAssy(
        std::shared_ptr<IR::FunctionDefinition> irFunctionDefinition,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    std::shared_ptr<Assembly::StaticVariable> convertIRStaticVariableToAssy(
        std::shared_ptr<IR::StaticVariable> irStaticVariable);
    void convertIRInstructionToAssy(
        std::shared_ptr<IR::Instruction> irInstruction,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRReturnInstructionToAssy(
        std::shared_ptr<IR::ReturnInstruction> returnInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRUnaryInstructionToAssy(
        std::shared_ptr<IR::UnaryInstruction> unaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRBinaryInstructionToAssy(
        std::shared_ptr<IR::BinaryInstruction> binaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRLabelInstructionToAssy(
        std::shared_ptr<IR::LabelInstruction> labelInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRJumpInstructionToAssy(
        std::shared_ptr<IR::JumpInstruction> jumpInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRJumpIfZeroInstructionToAssy(
        std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRJumpIfNotZeroInstructionToAssy(
        std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRCopyInstructionToAssy(
        std::shared_ptr<IR::CopyInstruction> copyInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRSignExtendInstructionToAssy(
        std::shared_ptr<IR::SignExtendInstruction> signExtendInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRTruncateInstructionToAssy(
        std::shared_ptr<IR::TruncateInstruction> truncateInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRFunctionCallInstructionToAssy(
        std::shared_ptr<IR::FunctionCallInstruction> functionCallInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    std::shared_ptr<Assembly::Operand>
    convertValue(std::shared_ptr<IR::Value> value);
    std::shared_ptr<Assembly::AssemblyType>
    determineAssemblyType(std::shared_ptr<IR::Value> value);

  public:
    static std::shared_ptr<Assembly::AssemblyType>
    convertASTTypeToAssemblyType(std::shared_ptr<AST::Type> astType);
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_GENERATOR_H
