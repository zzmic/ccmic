#ifndef ASSEMBLY_GENERATOR_H
#define ASSEMBLY_GENERATOR_H

#include "assembly.h"
#include "ir.h"
#include "types.h"
#include <unordered_map>

namespace Assembly {
class AssemblyGenerator {
  public:
    AssemblyGenerator(
        std::unordered_map<std::string, std::pair<std::shared_ptr<Type>, bool>>
            symbols);
    std::shared_ptr<Assembly::Program>
    generate(std::shared_ptr<IR::Program> irProgram);

  private:
    std::unordered_map<std::string, std::pair<std::shared_ptr<Type>, bool>>
        symbols;
    void generateAssyInstruction(
        std::shared_ptr<IR::Instruction> irInstruction,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyReturnInstruction(
        std::shared_ptr<IR::ReturnInstruction> returnInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyUnaryInstruction(
        std::shared_ptr<IR::UnaryInstruction> unaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyBinaryInstruction(
        std::shared_ptr<IR::BinaryInstruction> binaryInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyLabelInstruction(
        std::shared_ptr<IR::LabelInstruction> labelInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyJumpInstruction(
        std::shared_ptr<IR::JumpInstruction> jumpInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyJumpIfZeroInstruction(
        std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyJumpIfNotZeroInstruction(
        std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void generateAssyCopyInstruction(
        std::shared_ptr<IR::CopyInstruction> copyInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    void convertIRFunctionCallInstructionToAssy(
        std::shared_ptr<IR::FunctionCallInstruction> functionCallInstr,
        std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
            instructions);
    std::shared_ptr<Assembly::Operand>
    convertValue(std::shared_ptr<IR::Value> value);
};
} // namespace Assembly

#endif // ASSEMBLY_GENERATOR_H
