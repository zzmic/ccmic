#ifndef MIDEND_IR_GENERATOR_H
#define MIDEND_IR_GENERATOR_H

#include "../frontend/block.h"
#include "../frontend/declaration.h"
#include "../frontend/expression.h"
#include "../frontend/function.h"
#include "../frontend/program.h"
#include "../frontend/statement.h"
#include "ir.h"
#include <optional>
#include <string_view>
#include <unordered_map>

namespace IR {
class IRGenerator {
  public:
    IRGenerator(
        int variableResolutionCounter,
        std::unordered_map<std::string,
                           std::pair<std::shared_ptr<AST::Type>,
                                     std::shared_ptr<AST::IdentifierAttribute>>>
            symbols);
    std::pair<std::shared_ptr<IR::Program>,
              std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
    generateIR(std::shared_ptr<AST::Program> astProgram);

  private:
    int irTemporariesCounter = 0;
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<AST::Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        symbols;
    void generateIRBlock(
        std::shared_ptr<AST::Block> astBlock,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRFunctionDefinition(
        std::shared_ptr<AST::FunctionDeclaration> astFunctionDeclaration,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRVariableDefinition(
        std::shared_ptr<AST::VariableDeclaration> astVariableDeclaration,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRStatement(
        std::shared_ptr<AST::Statement> astStatement,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRReturnStatement(
        std::shared_ptr<AST::ReturnStatement> returnStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRExpressionStatement(
        std::shared_ptr<AST::ExpressionStatement> expressionStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRIfStatement(
        std::shared_ptr<AST::IfStatement> ifStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRBreakStatement(
        std::shared_ptr<AST::BreakStatement> breakStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRContinueStatement(
        std::shared_ptr<AST::ContinueStatement> continueStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRWhileStatement(
        std::shared_ptr<AST::WhileStatement> whileStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRDoWhileStatement(
        std::shared_ptr<AST::DoWhileStatement> doWhileStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRForStatement(
        std::shared_ptr<AST::ForStatement> forStmt,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::Value> generateIRInstruction(
        std::shared_ptr<AST::Expression> e,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRUnaryInstruction(
        std::shared_ptr<AST::UnaryExpression> unaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRBinaryInstruction(
        std::shared_ptr<AST::BinaryExpression> binaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRInstructionWithLogicalAnd(
        std::shared_ptr<AST::BinaryExpression> binaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRInstructionWithLogicalOr(
        std::shared_ptr<AST::BinaryExpression> binaryExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRCopyInstruction(
        std::shared_ptr<IR::Value> src, std::shared_ptr<IR::Value> dst,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRJumpInstruction(
        std::string_view target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRJumpIfZeroInstruction(
        std::shared_ptr<IR::Value> condition, std::string_view target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRJumpIfNotZeroInstruction(
        std::shared_ptr<IR::Value> condition, std::string_view target,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    void generateIRLabelInstruction(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRFunctionCallInstruction(
        std::string_view functionIdentifier,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Value>>> args,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::shared_ptr<IR::VariableValue> generateIRCastInstruction(
        std::shared_ptr<AST::CastExpression> castExpr,
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            instructions);
    std::string generateIRTemporary();
    std::string generateIRFalseLabel();
    std::string generateIRTrueLabel();
    std::string generateIRResultLabel();
    std::string generateIREndLabel();
    std::string generateIRElseLabel();
    std::string generateIRE2Label();
    std::string
    generateIRContinueLoopLabel(std::string_view loopLabelingLabel) const;
    std::string
    generateIRBreakLoopLabel(std::string_view loopLabelingLabel) const;
    std::string generateIRStartLabel() const;
    std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
    convertSymbolsToIRStaticVariables();
    std::shared_ptr<IR::UnaryOperator>
    convertUnop(std::shared_ptr<AST::UnaryOperator> op);
    std::shared_ptr<IR::BinaryOperator>
    convertBinop(std::shared_ptr<AST::BinaryOperator> op);
};
} // namespace IR

#endif // MIDEND_IR_GENERATOR_H
