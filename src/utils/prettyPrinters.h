#ifndef UTILS_PRETTY_PRINTERS_H
#define UTILS_PRETTY_PRINTERS_H

#include "../backend/assembly.h"
#include "../midend/ir.h"
#include <iostream>

class PrettyPrinters {
  public:
    static void printIRProgram(
        const std::shared_ptr<IR::Program> &irProgram,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
            &irStaticVariables);
    static void printAssemblyProgram(
        const std::shared_ptr<Assembly::Program> &assemblyProgram);

  private:
    /* Auxiliary functions for printing the IR program. */
    static void printIRFunctionDefinition(
        const std::shared_ptr<IR::FunctionDefinition> &functionDefinition);
    static void printIRStaticVariable(
        const std::shared_ptr<IR::StaticVariable> &staticVariable);
    static void
    printIRInstruction(const std::shared_ptr<IR::Instruction> &instruction);
    static void printIRReturnInstruction(
        const std::shared_ptr<IR::ReturnInstruction> &returnInstruction);
    static void printIRSignExtendInstruction(
        const std::shared_ptr<IR::SignExtendInstruction>
            &signExtendInstruction);
    static void printIRTruncateInstruction(
        const std::shared_ptr<IR::TruncateInstruction> &truncateInstruction);
    static void printIRUnaryInstruction(
        const std::shared_ptr<IR::UnaryInstruction> &unaryInstruction);
    static void printIRBinaryInstruction(
        const std::shared_ptr<IR::BinaryInstruction> &binaryInstruction);
    static void printIRCopyInstruction(
        const std::shared_ptr<IR::CopyInstruction> &copyInstruction);
    static void printIRJumpInstruction(
        const std::shared_ptr<IR::JumpInstruction> &jumpInstruction);
    static void printIRJumpIfZeroInstruction(
        const std::shared_ptr<IR::JumpIfZeroInstruction>
            &jumpIfZeroInstruction);
    static void printIRJumpIfNotZeroInstruction(
        const std::shared_ptr<IR::JumpIfNotZeroInstruction>
            &jumpIfNotZeroInstruction);
    static void printIRLabelInstruction(
        const std::shared_ptr<IR::LabelInstruction> &labelInstruction);
    static void printIRFunctionCallInstruction(
        const std::shared_ptr<IR::FunctionCallInstruction>
            &functionCallInstruction);

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
    static void printAssyRetInstruction();
    static void printAssyAllocateStackInstruction(
        const std::shared_ptr<Assembly::AllocateStackInstruction>
            &allocateStackInstruction);
    static void printAssyDeallocateStackInstruction(
        const std::shared_ptr<Assembly::DeallocateStackInstruction>
            &deallocateStackInstruction);
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
    static void printAssyCdqInstruction();
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
