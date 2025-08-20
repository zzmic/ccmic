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
    explicit IRGenerator(int variableResolutionCounter);
    [[nodiscard]] std::pair<std::unique_ptr<IR::Program>,
                            std::vector<std::unique_ptr<IR::StaticVariable>>>
    generateIR(const std::unique_ptr<AST::Program> &astProgram);

  private:
    int irTemporariesCounter = 0;
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>> generateIRBlock(
        std::unique_ptr<AST::Block> &astBlock,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRFunctionDefinition(
        std::unique_ptr<AST::FunctionDeclaration> &astFunctionDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRVariableDefinition(
        std::unique_ptr<AST::VariableDeclaration> &astVariableDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRStatement(
        std::unique_ptr<AST::Statement> &astStatement,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRReturnStatement(
        std::unique_ptr<AST::ReturnStatement> &returnStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRExpressionStatement(
        std::unique_ptr<AST::ExpressionStatement> &expressionStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRIfStatement(
        std::unique_ptr<AST::IfStatement> &ifStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRBreakStatement(
        std::unique_ptr<AST::BreakStatement> &breakStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRContinueStatement(
        std::unique_ptr<AST::ContinueStatement> &continueStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRWhileStatement(
        std::unique_ptr<AST::WhileStatement> &whileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRDoWhileStatement(
        std::unique_ptr<AST::DoWhileStatement> &doWhileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRForStatement(
        std::unique_ptr<AST::ForStatement> &forStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::Value>
    generateIRInstruction(std::unique_ptr<AST::Expression> &e);
    [[nodiscard]] std::unique_ptr<IR::VariableValue> generateIRUnaryInstruction(
        std::unique_ptr<AST::UnaryExpression> &unaryExpr);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRBinaryInstruction(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalAnd(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalOr(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRCopyInstruction(std::unique_ptr<IR::Value> src,
                              std::unique_ptr<IR::Value> dst);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRJumpInstruction(std::string_view target);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRJumpIfZeroInstruction(std::unique_ptr<IR::Value> condition,
                                    std::string_view target);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRJumpIfNotZeroInstruction(std::unique_ptr<IR::Value> condition,
                                       std::string_view target);
    [[nodiscard]] std::vector<std::unique_ptr<IR::Instruction>>
    generateIRLabelInstruction(std::string_view identifier);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRFunctionCallInstruction(
        std::string_view functionIdentifier,
        std::vector<std::unique_ptr<IR::Value>> &args);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRCastInstruction(std::unique_ptr<AST::CastExpression> &castExpr);
    [[nodiscard]] std::string generateIRTemporary();
    [[nodiscard]] std::string generateIRFalseLabel();
    [[nodiscard]] std::string generateIRTrueLabel();
    [[nodiscard]] std::string generateIRResultLabel();
    [[nodiscard]] std::string generateIREndLabel();
    [[nodiscard]] std::string generateIRElseLabel();
    [[nodiscard]] std::string generateIRE2Label();
    [[nodiscard]] std::string
    generateIRContinueLoopLabel(std::string_view loopLabelingLabel) const;
    [[nodiscard]] std::string
    generateIRBreakLoopLabel(std::string_view loopLabelingLabel) const;
    [[nodiscard]] std::string generateIRStartLabel() const;
    [[nodiscard]]
    std::vector<std::unique_ptr<IR::StaticVariable>>
    convertFrontendSymbolTableToIRStaticVariables();
    [[nodiscard]] std::unique_ptr<IR::UnaryOperator>
    convertUnop(std::unique_ptr<AST::UnaryOperator> &op);
    [[nodiscard]] std::unique_ptr<IR::BinaryOperator>
    convertBinop(std::unique_ptr<AST::BinaryOperator> &op);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRVariable(std::unique_ptr<AST::BinaryExpression> &binaryExpr);
};
} // namespace IR

#endif // MIDEND_IR_GENERATOR_H
