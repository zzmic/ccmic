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
    static void
    printCopyInstruction(std::shared_ptr<IR::CopyInstruction> copyInstruction);
    static void printIRJumpInstruction(
        std::shared_ptr<IR::JumpInstruction> jumpInstruction);
    static void printIRJumpIfZeroInstruction(
        std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstruction);
    static void printIRJumpIfNotZeroInstruction(
        std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstruction);
    static void printIRLabelInstruction(
        std::shared_ptr<IR::LabelInstruction> labelInstruction);
};

#endif // PRETTY_PRINTERS_H
