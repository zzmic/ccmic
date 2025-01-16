#ifndef PIPELINE_STAGES_EXECUTORS_H
#define PIPELINE_STAGES_EXECUTORS_H

#include "backend/assembly.h"
#include "backend/assemblyGenerator.h"
#include "backend/fixupPass.h"
#include "backend/pseudoToStackPass.h"
#include "compilerDriver.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/printVisitor.h"
#include "frontend/semanticAnalysisPasses.h"
#include "midend/ir.h"
#include "midend/irGenerator.h"
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
    static std::pair<
        int,
        std::unordered_map<
            std::string, std::pair<std::shared_ptr<Type>,
                                   std::shared_ptr<AST::IdentifierAttribute>>>>
    semanticAnalysisExecutor(std::shared_ptr<AST::Program> astProgram);
    static std::pair<
        std::shared_ptr<IR::Program>,
        std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
    irGeneratorExecutor(
        std::shared_ptr<AST::Program> astProgram, int variableResolutionCounter,
        std::unordered_map<std::string,
                           std::pair<std::shared_ptr<Type>,
                                     std::shared_ptr<AST::IdentifierAttribute>>>
            symbols);
    static std::shared_ptr<Assembly::Program> codegenExecutor(
        std::shared_ptr<IR::Program> irProgram,
        std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
            irStaticVariables,
        std::unordered_map<std::string,
                           std::pair<std::shared_ptr<Type>,
                                     std::shared_ptr<AST::IdentifierAttribute>>>
            symbols);
    static void
    codeEmissionExecutor(std::shared_ptr<Assembly::Program> assemblyProgram,
                         const std::string &assemblyFile);

  private:
    // Auxiliary functions for emitting the assembly code (to the assembly
    // file).
    static void emitAssyFunctionDefinition(
        std::shared_ptr<Assembly::FunctionDefinition> functionDefinition,
        std::ofstream &assemblyFileStream);
    static void emitAssyStaticVariable(
        std::shared_ptr<Assembly::StaticVariable> staticVariable,
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
    static void emitAssyDeallocateStackInstruction(
        std::shared_ptr<Assembly::DeallocateStackInstruction>
            deallocateStackInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyPushInstruction(
        std::shared_ptr<Assembly::PushInstruction> pushInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCallInstruction(
        std::shared_ptr<Assembly::CallInstruction> callInstruction,
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
