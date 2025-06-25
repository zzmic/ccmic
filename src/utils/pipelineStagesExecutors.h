#ifndef UTILS_PIPELINE_STAGES_EXECUTORS_H
#define UTILS_PIPELINE_STAGES_EXECUTORS_H

#include "../backend/assembly.h"
#include "../backend/assemblyGenerator.h"
#include "../backend/fixupPass.h"
#include "../backend/pseudoToStackPass.h"
#include "../frontend/lexer.h"
#include "../frontend/parser.h"
#include "../frontend/printVisitor.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../midend/ir.h"
#include "../midend/irGenerator.h"
#include "../midend/irOptimizationPasses.h"
#include "compilerDriver.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string_view>

class PipelineStagesExecutors {
  public:
    static std::vector<Token> lexerExecutor(std::string_view sourceFile);
    static std::shared_ptr<AST::Program>
    parserExecutor(const std::vector<Token> &tokens);
    static std::pair<
        int,
        std::unordered_map<
            std::string, std::pair<std::shared_ptr<AST::Type>,
                                   std::shared_ptr<AST::IdentifierAttribute>>>>
    semanticAnalysisExecutor(const std::shared_ptr<AST::Program> &astProgram);
    static std::pair<
        std::shared_ptr<IR::Program>,
        std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
    irGeneratorExecutor(
        const std::shared_ptr<AST::Program> &astProgram,
        int variableResolutionCounter,
        const std::unordered_map<
            std::string, std::pair<std::shared_ptr<AST::Type>,
                                   std::shared_ptr<AST::IdentifierAttribute>>>
            &symbols);
    static void
    irOptimizationExecutor(const std::shared_ptr<IR::Program> &irProgram,
                           bool foldConstantsPass, bool propagateCopiesPass,
                           bool eliminateUnreachableCodePass,
                           bool eliminateDeadStoresPass);
    static std::shared_ptr<Assembly::Program> codegenExecutor(
        const std::shared_ptr<IR::Program> &irProgram,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
            &irStaticVariables,
        const std::unordered_map<
            std::string, std::pair<std::shared_ptr<AST::Type>,
                                   std::shared_ptr<AST::IdentifierAttribute>>>
            &symbols);
    static void codeEmissionExecutor(
        const std::shared_ptr<Assembly::Program> &assemblyProgram,
        std::string_view assemblyFile);

  private:
    // Auxiliary functions for emitting the assembly code (to the assembly
    // file).
    static void emitAssyFunctionDefinition(
        const std::shared_ptr<Assembly::FunctionDefinition> &functionDefinition,
        std::ofstream &assemblyFileStream);
    static void emitAssyStaticVariable(
        const std::shared_ptr<Assembly::StaticVariable> &staticVariable,
        std::ofstream &assemblyFileStream);
    static void emitAssyInstruction(
        const std::shared_ptr<Assembly::Instruction> &instruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyMovInstruction(
        const std::shared_ptr<Assembly::MovInstruction> &movInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyRetInstruction(std::ofstream &assemblyFileStream);
    static void emitAssyAllocateStackInstruction(
        const std::shared_ptr<Assembly::AllocateStackInstruction>
            &allocateStackInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyDeallocateStackInstruction(
        const std::shared_ptr<Assembly::DeallocateStackInstruction>
            &deallocateStackInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyPushInstruction(
        const std::shared_ptr<Assembly::PushInstruction> &pushInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCallInstruction(
        const std::shared_ptr<Assembly::CallInstruction> &callInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyUnaryInstruction(
        const std::shared_ptr<Assembly::UnaryInstruction> &unaryInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyBinaryInstruction(
        const std::shared_ptr<Assembly::BinaryInstruction> &binaryInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCmpInstruction(
        const std::shared_ptr<Assembly::CmpInstruction> &cmpInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyIdivInstruction(
        const std::shared_ptr<Assembly::IdivInstruction> &idivInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyCdqInstruction(std::ofstream &assemblyFileStream);
    static void emitAssyJmpInstruction(
        const std::shared_ptr<Assembly::JmpInstruction> &jmpInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyJmpCCInstruction(
        const std::shared_ptr<Assembly::JmpCCInstruction> &jmpCCInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssySetCCInstruction(
        const std::shared_ptr<Assembly::SetCCInstruction> &setCCInstruction,
        std::ofstream &assemblyFileStream);
    static void emitAssyLabelInstruction(
        const std::shared_ptr<Assembly::LabelInstruction> &labelInstruction,
        std::ofstream &assemblyFileStream);
    static void prependUnderscoreToIdentifierIfMacOS(
        [[maybe_unused]] std::string &identifier);
};

#endif // UTILS_PIPELINE_STAGES_EXECUTORS_H
