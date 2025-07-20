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
    [[nodiscard]] std::pair<
        std::shared_ptr<IR::Program>,
        std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>>
    generateIR(const std::shared_ptr<AST::Program> &astProgram);

  private:
    int irTemporariesCounter = 0;
    void generateIRBlock(
        const std::shared_ptr<AST::Block> &astBlock,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRFunctionDefinition(
        const std::shared_ptr<AST::FunctionDeclaration> &astFunctionDeclaration,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRVariableDefinition(
        const std::shared_ptr<AST::VariableDeclaration> &astVariableDeclaration,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRStatement(
        const std::shared_ptr<AST::Statement> &astStatement,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRReturnStatement(
        const std::shared_ptr<AST::ReturnStatement> &returnStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRExpressionStatement(
        const std::shared_ptr<AST::ExpressionStatement> &expressionStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRIfStatement(
        const std::shared_ptr<AST::IfStatement> &ifStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRBreakStatement(
        const std::shared_ptr<AST::BreakStatement> &breakStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRContinueStatement(
        const std::shared_ptr<AST::ContinueStatement> &continueStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRWhileStatement(
        const std::shared_ptr<AST::WhileStatement> &whileStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRDoWhileStatement(
        const std::shared_ptr<AST::DoWhileStatement> &doWhileStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRForStatement(
        const std::shared_ptr<AST::ForStatement> &forStmt,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::Value> generateIRInstruction(
        const std::shared_ptr<AST::Expression> &e,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue> generateIRUnaryInstruction(
        const std::shared_ptr<AST::UnaryExpression> &unaryExpr,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue>
    generateIRBinaryInstruction(
        const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalAnd(
        const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalOr(
        const std::shared_ptr<AST::BinaryExpression> &binaryExpr,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRCopyInstruction(
        const std::shared_ptr<IR::Value> &src,
        const std::shared_ptr<IR::Value> &dst,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRJumpInstruction(
        std::string_view target,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRJumpIfZeroInstruction(
        const std::shared_ptr<IR::Value> &condition, std::string_view target,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRJumpIfNotZeroInstruction(
        const std::shared_ptr<IR::Value> &condition, std::string_view target,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    void generateIRLabelInstruction(
        std::string_view identifier,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue>
    generateIRFunctionCallInstruction(
        std::string_view functionIdentifier,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Value>>> &args,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
    [[nodiscard]] std::shared_ptr<IR::VariableValue> generateIRCastInstruction(
        const std::shared_ptr<AST::CastExpression> &castExpr,
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &instructions);
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
    [[nodiscard]] std::shared_ptr<
        std::vector<std::shared_ptr<IR::StaticVariable>>>
    convertFrontendSymbolTableToIRStaticVariables();
    [[nodiscard]] std::shared_ptr<IR::UnaryOperator>
    convertUnop(const std::shared_ptr<AST::UnaryOperator> &op);
    [[nodiscard]] std::shared_ptr<IR::BinaryOperator>
    convertBinop(const std::shared_ptr<AST::BinaryOperator> &op);
    [[nodiscard]] std::shared_ptr<IR::VariableValue> generateIRVariable(
        const std::shared_ptr<AST::BinaryExpression> &binaryExpr);
};
} // namespace IR

#endif // MIDEND_IR_GENERATOR_H
