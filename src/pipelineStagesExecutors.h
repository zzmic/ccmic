#ifndef PIPELINE_STAGES_EXECUTORS_H
#define PIPELINE_STAGES_EXECUTORS_H

#include "assembly.h"
#include "assemblyGenerator.cpp"
#include "compilerDriver.h"
#include "fixupPass.h"
#include "ir.h"
#include "irGenerator.cpp"
#include "lexer.h"
#include "parser.h"
#include "printVisitor.h"
#include "pseudoToStackPass.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

class PipelineStagesExecutors {
  public:
    static std::vector<Token> lexerExecutor(const std::string &sourceFile);
    static std::shared_ptr<AST::Program>
    parserExecutor(const std::vector<Token> &tokens);
    static std::shared_ptr<IR::Program>
    irGeneratorExecutor(std::shared_ptr<AST::Program> astProgram);
    static std::shared_ptr<Assembly::Program>
    codegenExecutor(std::shared_ptr<IR::Program> irProgram);
    static void
    codeEmissionExecutor(std::shared_ptr<Assembly::Program> assemblyProgram,
                         const std::string &assemblyFile);

  private:
    // Auxiliary functions for emitting the assembly code.
    static void emitAssyFunctionDefinition(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition,
        std::ofstream &assemblyFileStream);
    static void
    emitAssyInstruction(std::shared_ptr<Assembly::Instruction> instruction,
                        std::ofstream &assemblyFileStream);
    static void emitAssyMovInstruction(
        std::shared_ptr<Assembly::MovInstruction> movInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyRetInstruction(std::ofstream &assemblyFileStream);
    static void emitAssyAllocateStackInstruction(
        std::shared_ptr<Assembly::AllocateStackInstruction>
            allocateStackInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyUnaryInstruction(
        std::shared_ptr<Assembly::UnaryInstruction> unaryInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyBinaryInstruction(
        std::shared_ptr<Assembly::BinaryInstruction> binaryInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCmpInstruction(
        std::shared_ptr<Assembly::CmpInstruction> cmpInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyIdivInstruction(
        std::shared_ptr<Assembly::IdivInstruction> idivInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCdqInstruction(std::ofstream &assemblyFileStream);
    static void emitAssyJmpInstruction(
        std::shared_ptr<Assembly::JmpInstruction> jmpInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyJmpCCInstruction(
        std::shared_ptr<Assembly::JmpCCInstruction> jmpCCInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssySetCCInstruction(
        std::shared_ptr<Assembly::SetCCInstruction> setCCInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyLabelInstruction(
        std::shared_ptr<Assembly::LabelInstruction> labelInstruction,
        std::ofstream &assemblyFileStream);
};

#endif // PIPELINE_STAGES_EXECUTORS_H
