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
    void generateIRBlock(
        AST::Block *astBlock,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRFunctionDefinition(
        std::unique_ptr<AST::FunctionDeclaration> &astFunctionDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRVariableDefinition(
        std::unique_ptr<AST::VariableDeclaration> &astVariableDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRStatement(
        std::unique_ptr<AST::Statement> &astStatement,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRReturnStatement(
        std::unique_ptr<AST::ReturnStatement> &returnStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRExpressionStatement(
        std::unique_ptr<AST::ExpressionStatement> &expressionStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRIfStatement(
        std::unique_ptr<AST::IfStatement> &ifStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRBreakStatement(
        std::unique_ptr<AST::BreakStatement> &breakStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRContinueStatement(
        std::unique_ptr<AST::ContinueStatement> &continueStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRWhileStatement(
        std::unique_ptr<AST::WhileStatement> &whileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRDoWhileStatement(
        std::unique_ptr<AST::DoWhileStatement> &doWhileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRForStatement(
        std::unique_ptr<AST::ForStatement> &forStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::Value> generateIRInstruction(
        std::unique_ptr<AST::Expression> &e,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue> generateIRUnaryInstruction(
        std::unique_ptr<AST::UnaryExpression> &unaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRBinaryInstruction(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalAnd(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalOr(
        std::unique_ptr<AST::BinaryExpression> &binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRCopyInstruction(
        std::unique_ptr<IR::Value> src, std::unique_ptr<IR::Value> dst,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRJumpInstruction(
        std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRJumpIfZeroInstruction(
        std::unique_ptr<IR::Value> condition, std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRJumpIfNotZeroInstruction(
        std::unique_ptr<IR::Value> condition, std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    void generateIRLabelInstruction(
        std::string_view identifier,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRFunctionCallInstruction(
        std::string_view functionIdentifier,
        std::vector<std::unique_ptr<IR::Value>> &args,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
    [[nodiscard]] std::unique_ptr<IR::VariableValue> generateIRCastInstruction(
        std::unique_ptr<AST::CastExpression> &castExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);
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
