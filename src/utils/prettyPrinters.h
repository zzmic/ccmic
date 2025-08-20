#ifndef UTILS_PRETTY_PRINTERS_H
#define UTILS_PRETTY_PRINTERS_H

#include "../backend/assembly.h"
#include "../midend/ir.h"
#include <iostream>

class PrettyPrinters {
  public:
    static void printIRProgram(
        std::unique_ptr<IR::Program> &irProgram,
        std::vector<std::unique_ptr<IR::StaticVariable>> &irStaticVariables);
    static void printAssemblyProgram(
        const std::shared_ptr<Assembly::Program> &assemblyProgram);

  private:
    /* Auxiliary functions for printing the IR program. */
    static void
    printIRFunctionDefinition(IR::FunctionDefinition *functionDefinition);
    static void printIRStaticVariable(IR::StaticVariable *staticVariable);
    static void printIRInstruction(IR::Instruction *instruction);
    static void
    printIRReturnInstruction(IR::ReturnInstruction *returnInstruction);
    static void printIRSignExtendInstruction(
        IR::SignExtendInstruction *signExtendInstruction);
    static void
    printIRTruncateInstruction(IR::TruncateInstruction *truncateInstruction);
    static void printIRUnaryInstruction(IR::UnaryInstruction *unaryInstruction);
    static void
    printIRBinaryInstruction(IR::BinaryInstruction *binaryInstruction);
    static void printIRCopyInstruction(IR::CopyInstruction *copyInstruction);
    static void printIRJumpInstruction(IR::JumpInstruction *jumpInstruction);
    static void printIRJumpIfZeroInstruction(
        IR::JumpIfZeroInstruction *jumpIfZeroInstruction);
    static void printIRJumpIfNotZeroInstruction(
        IR::JumpIfNotZeroInstruction *jumpIfNotZeroInstruction);
    static void printIRLabelInstruction(IR::LabelInstruction *labelInstruction);
    static void printIRFunctionCallInstruction(
        IR::FunctionCallInstruction *functionCallInstruction);

    /* Auxiliary functions for printing the assembly program. */
    static void printAssyFunctionDefinition(
        const std::shared_ptr<Assembly::FunctionDefinition>
            &functionDefinition);
    static void printAssyStaticVariable(
        const std::shared_ptr<Assembly::StaticVariable> &staticVariable);
    static void printAssyInstruction(
        const std::shared_ptr<Assembly::Instruction> &instruction);
    static void printAssyMovInstruction(
        const std::shared_ptr<Assembly::MovInstruction> &movInstruction);
    static void printAssyMovsxInstruction(
        const std::shared_ptr<Assembly::MovsxInstruction> &movsxInstruction);
    static void printAssyRetInstruction(
        const std::shared_ptr<Assembly::RetInstruction> &retInstruction);
    static void printAssyPushInstruction(
        const std::shared_ptr<Assembly::PushInstruction> &pushInstruction);
    static void printAssyCallInstruction(
        const std::shared_ptr<Assembly::CallInstruction> &callInstruction);
    static void printAssyUnaryInstruction(
        const std::shared_ptr<Assembly::UnaryInstruction> &unaryInstruction);
    static void printAssyBinaryInstruction(
        const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction);
    static void printAssyCmpInstruction(
        const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction);
    static void printAssyIdivInstruction(
        const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction);
    static void printAssyCdqInstruction(
        const std::shared_ptr<Assembly::CdqInstruction> &cdqInstruction);
    static void printAssyJmpInstruction(
        const std::shared_ptr<Assembly::JmpInstruction> &jmpInstruction);
    static void printAssyJmpCCInstruction(
        const std::shared_ptr<Assembly::JmpCCInstruction> &jmpCCInstruction);
    static void printAssySetCCInstruction(
        const std::shared_ptr<Assembly::SetCCInstruction> &setCCInstruction);
    static void printAssyLabelInstruction(
        const std::shared_ptr<Assembly::LabelInstruction> &labelInstruction);
    static void prependUnderscoreToIdentifierIfMacOS(
        [[maybe_unused]] std::string &identifier);
};

#endif // UTILS_PRETTY_PRINTERS_H
