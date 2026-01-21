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
#include <memory>
#include <sstream>
#include <string.h>
#include <string_view>

/**
 * Class containing static executor methods for each pipeline stage.
 */
class PipelineStagesExecutors {
  public:
    /**
     * Perform lexical analysis on the source file and generate the list of
     * tokens.
     *
     * @param sourceFile The input source file.
     * @return The list of tokens generated from lexing.
     */
    [[nodiscard]] static std::vector<Token>
    lexerExecutor(std::string_view sourceFile);

    /**
     * Perform syntactic analysis on the list of tokens and generate the AST
     * program.
     *
     * @param tokens The list of tokens to parse.
     * @return The AST program generated from parsing.
     */
    [[nodiscard]] static std::unique_ptr<AST::Program>
    parserExecutor(const std::vector<Token> &tokens);

    /**
     * Perform semantic-analysis passes on the AST program.
     *
     * @param astProgram The AST program to analyze.
     * @param frontendSymbolTable The frontend symbol table.
     * @return An integer counter for variable resolution.
     */
    [[nodiscard]] static int
    semanticAnalysisExecutor(AST::Program &astProgram,
                             AST::FrontendSymbolTable &frontendSymbolTable);

    /**
     * Generate (but not yet emit) the IR from the AST program.
     *
     * @param astProgram The AST program to convert to IR.
     * @param variableResolutionCounter An integer counter for variable
     * resolution.
     * @param frontendSymbolTable The frontend symbol table.
     * @return A pair consisting of the IR program and the vector of static
     * variables in IR.
     */
    [[nodiscard]] static std::pair<
        std::unique_ptr<IR::Program>,
        std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>>
    irGeneratorExecutor(const AST::Program &astProgram,
                        int variableResolutionCounter,
                        AST::FrontendSymbolTable &frontendSymbolTable);

    /**
     * Perform optimization passes on the IR program.
     *
     * @param irProgram The IR program to optimize.
     * @param foldConstantsPass Whether to perform the constant-folding pass.
     * @param propagateCopiesPass Whether to perform the copy-propagation pass.
     * @param eliminateUnreachableCodePass Whether to perform the
     * unreachable-code elimination pass.
     * @param eliminateDeadStoresPass Whether to perform the dead-store
     * elimination pass.
     */
    static void irOptimizationExecutor(IR::Program &irProgram,
                                       bool foldConstantsPass,
                                       bool propagateCopiesPass,
                                       bool eliminateUnreachableCodePass,
                                       bool eliminateDeadStoresPass);

    /**
     * Generate (but not yet emit) the assembly program from the IR program.
     *
     * @param irProgram The IR program to convert to assembly.
     * @param irStaticVariables A vector of static variables in IR.
     * @param frontendSymbolTable The frontend symbol table.
     * @return The assembly program generated from the IR.
     */
    [[nodiscard]] static std::unique_ptr<Assembly::Program>
    codegenExecutor(const IR::Program &irProgram,
                    const std::vector<std::unique_ptr<IR::StaticVariable>>
                        &irStaticVariables,
                    const AST::FrontendSymbolTable &frontendSymbolTable);

    /**
     * Emit the generated assembly code to the assembly file.
     *
     * @param assemblyProgram The assembly program to emit.
     * @param assemblyFile The output assembly file.
     */
    static void codeEmissionExecutor(const Assembly::Program &assemblyProgram,
                                     std::string_view assemblyFile);

  private:
    /**
     * Emit the assembly code for a function definition.
     *
     * @param functionDefinition The function definition to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void emitAssyFunctionDefinition(
        const Assembly::FunctionDefinition &functionDefinition,
        std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a static variable.
     *
     * @param staticVariable The static variable to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyStaticVariable(const Assembly::StaticVariable &staticVariable,
                           std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for an instruction.
     *
     * @param instruction The instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void emitAssyInstruction(const Assembly::Instruction &instruction,
                                    std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a move instruction.
     *
     * @param movInstruction The move instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyMovInstruction(const Assembly::MovInstruction &movInstruction,
                           std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a move-with-sign-extend instruction.
     *
     * @param movsxInstruction The move-with-sign-extend instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyMovsxInstruction(const Assembly::MovsxInstruction &movsxInstruction,
                             std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a return instruction.
     *
     * @param assemblyFileStream The output assembly file stream.
     */
    static void emitAssyRetInstruction(std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a push instruction.
     *
     * @param pushInstruction The push instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyPushInstruction(const Assembly::PushInstruction &pushInstruction,
                            std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a call instruction.
     *
     * @param callInstruction The call instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyCallInstruction(const Assembly::CallInstruction &callInstruction,
                            std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a unary instruction.
     *
     * @param unaryInstruction The unary instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyUnaryInstruction(const Assembly::UnaryInstruction &unaryInstruction,
                             std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a binary instruction.
     *
     * @param binaryInstruction The binary instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void emitAssyBinaryInstruction(
        const Assembly::BinaryInstruction &binaryInstruction,
        std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a compare instruction.
     *
     * @param cmpInstruction The compare instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyCmpInstruction(const Assembly::CmpInstruction &cmpInstruction,
                           std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a signed-integer-division instruction.
     *
     * @param idivInstruction The signed-integer-division instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyIdivInstruction(const Assembly::IdivInstruction &idivInstruction,
                            std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a covert-doubleword-to-quadword instruction.
     *
     * @param cdqInstruction The covert-doubleword-to-quadword instruction to
     * emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyCdqInstruction(const Assembly::CdqInstruction &cdqInstruction,
                           std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a jump instruction.
     *
     * @param jmpInstruction The jump instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyJmpInstruction(const Assembly::JmpInstruction &jmpInstruction,
                           std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a conditional jump instruction.
     *
     * @param jmpCCInstruction The conditional jump instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyJmpCCInstruction(const Assembly::JmpCCInstruction &jmpCCInstruction,
                             std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a set-byte-on-condition instruction.
     *
     * @param setCCInstruction The set-byte-on-condition instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssySetCCInstruction(const Assembly::SetCCInstruction &setCCInstruction,
                             std::ofstream &assemblyFileStream);

    /**
     * Emit the assembly code for a label instruction.
     *
     * @param labelInstruction The label instruction to emit.
     * @param assemblyFileStream The output assembly file stream.
     */
    static void
    emitAssyLabelInstruction(const Assembly::LabelInstruction &labelInstruction,
                             std::ofstream &assemblyFileStream);

    /**
     * Prepend an underscore to the identifier if the underlying OS is macOS.
     *
     * @param identifier The identifier to modify.
     */
    static void prependUnderscoreToIdentifierIfMacOS(
        [[maybe_unused]] std::string &identifier);
};

#endif // UTILS_PIPELINE_STAGES_EXECUTORS_H
