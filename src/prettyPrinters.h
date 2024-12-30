#ifndef PRETTY_PRINTERS_H
#define PRETTY_PRINTERS_H

#include "assembly.h"
#include "ir.h"
#include <iostream>

class PrettyPrinters {
  public:
    static void printIRProgram(std::shared_ptr<IR::Program> irProgram);
    static void
    printAssemblyProgram(std::shared_ptr<Assembly::Program> assemblyProgram);

  private:
    // Auxiliary functions for printing the IR program.
    static void printIRFunctionDefinition(
        std::shared_ptr<IR::FunctionDefinition> functionDefinition);
    static void
    printIRInstruction(std::shared_ptr<IR::Instruction> instruction);
    static void printIRReturnInstruction(
        std::shared_ptr<IR::ReturnInstruction> returnInstruction);
    static void printIRUnaryInstruction(
        std::shared_ptr<IR::UnaryInstruction> unaryInstruction);
    static void printIRBinaryInstruction(
        std::shared_ptr<IR::BinaryInstruction> binaryInstruction);
    static void printIRCopyInstruction(
        std::shared_ptr<IR::CopyInstruction> copyInstruction);
    static void printIRJumpInstruction(
        std::shared_ptr<IR::JumpInstruction> jumpInstruction);
    static void printIRJumpIfZeroInstruction(
        std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstruction);
    static void printIRJumpIfNotZeroInstruction(
        std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstruction);
    static void printIRLabelInstruction(
        std::shared_ptr<IR::LabelInstruction> labelInstruction);

    // Auxiliary functions for printing the assembly program.
    static void printAssyFunctionDefinition(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition);
    static void
    printAssyInstruction(std::shared_ptr<Assembly::Instruction> instruction);
    static void printAssyMovInstruction(
        std::shared_ptr<Assembly::MovInstruction> movInstruction);
    static void printAssyRetInstruction();
    static void printAssyAllocateStackInstruction(
        std::shared_ptr<Assembly::AllocateStackInstruction>
            allocateStackInstruction);
    static void printAssyUnaryInstruction(
        std::shared_ptr<Assembly::UnaryInstruction> unaryInstruction);
    static void printAssyBinaryInstruction(
        std::shared_ptr<Assembly::BinaryInstruction> binaryInstruction);
    static void printAssyCmpInstruction(
        std::shared_ptr<Assembly::CmpInstruction> cmpInstruction);
    static void printAssyIdivInstruction(
        std::shared_ptr<Assembly::IdivInstruction> idivInstruction);
    static void printAssyCdqInstruction();
    static void printAssyJmpInstruction(
        std::shared_ptr<Assembly::JmpInstruction> jmpInstruction);
    static void printAssyJmpCCInstruction(
        std::shared_ptr<Assembly::JmpCCInstruction> jmpCCInstruction);
    static void printAssySetCCInstruction(
        std::shared_ptr<Assembly::SetCCInstruction> setCCInstruction);
    static void printAssyLabelInstruction(
        std::shared_ptr<Assembly::LabelInstruction> labelInstruction);
};

#endif // PRETTY_PRINTERS_H
