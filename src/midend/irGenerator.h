#ifndef MIDEND_IR_GENERATOR_H
#define MIDEND_IR_GENERATOR_H

#include "../frontend/block.h"
#include "../frontend/declaration.h"
#include "../frontend/expression.h"
#include "../frontend/frontendSymbolTable.h"
#include "../frontend/function.h"
#include "../frontend/program.h"
#include "../frontend/statement.h"
#include "ir.h"
#include <optional>
#include <string_view>
#include <unordered_map>

namespace IR {
/**
 * Class for generating the corresponding intermediate representation (IR) from
 * the abstract syntax tree (AST).
 */
class IRGenerator {
  public:
    /**
     * Constructor for the `IRGenerator`.
     *
     * @param variableResolutionCounter The counter used for generating unique
     * variable names.
     * @param frontendSymbolTable The frontend symbol table.
     */
    explicit IRGenerator(int variableResolutionCounter,
                         AST::FrontendSymbolTable &frontendSymbolTable);

    /**
     * Generate the IR from the AST program.
     *
     * @param astProgram The AST program to generate the IR from.
     * @return A pair consisting of (a unique pointer to) the IR program and (a
     * unique pointer to) the vector of static variables in IR.
     */
    [[nodiscard]] std::pair<
        std::unique_ptr<IR::Program>,
        std::unique_ptr<std::vector<std::unique_ptr<IR::StaticVariable>>>>
    generateIR(const AST::Program &astProgram);

  private:
    /**
     * The counter used for generating unique temporary variable identifiers.
     */
    int irTemporariesCounter = 0;

    /**
     * The frontend symbol table.
     */
    AST::FrontendSymbolTable &frontendSymbolTable;

    /**
     * Generate the IR for a block.
     *
     * @param astBlock The AST node representing the block.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRBlock(
        const AST::Block *astBlock,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a function definition.
     *
     * @param astFunctionDeclaration The AST node representing the function
     * declaration.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRFunctionDefinition(
        const AST::FunctionDeclaration *astFunctionDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a variable definition.
     *
     * @param astVariableDeclaration The AST node representing the variable
     * declaration.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRVariableDefinition(
        const AST::VariableDeclaration *astVariableDeclaration,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a statement.
     *
     * @param astStatement The AST node representing the statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRStatement(
        const AST::Statement *astStatement,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a return statement.
     *
     * @param returnStmt The AST node representing the return statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRReturnStatement(
        const AST::ReturnStatement *returnStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for an expression statement.
     *
     * @param expressionStmt The AST node representing the expression statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRExpressionStatement(
        const AST::ExpressionStatement *expressionStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for an if statement.
     *
     * @param ifStmt The AST node representing the if statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRIfStatement(
        const AST::IfStatement *ifStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a break statement.
     *
     * @param breakStmt The AST node representing the break statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRBreakStatement(
        const AST::BreakStatement *breakStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a continue statement.
     *
     * @param continueStmt The AST node representing the continue statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRContinueStatement(
        const AST::ContinueStatement *continueStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a while statement.
     *
     * @param whileStmt The AST node representing the while statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRWhileStatement(
        const AST::WhileStatement *whileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a do-while statement.
     *
     * @param doWhileStmt The AST node representing the do-while statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRDoWhileStatement(
        const AST::DoWhileStatement *doWhileStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a for statement.
     *
     * @param forStmt The AST node representing the for statement.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRForStatement(
        const AST::ForStatement *forStmt,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for an expression.
     *
     * @param e The AST node representing the expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR value.
     */
    [[nodiscard]] std::unique_ptr<IR::Value> generateIRInstruction(
        const AST::Expression *e,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a unary expression.
     *
     * @param unaryExpr The AST node representing the unary expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue> generateIRUnaryInstruction(
        const AST::UnaryExpression *unaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a binary expression.
     *
     * @param binaryExpr The AST node representing the binary expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRBinaryInstruction(
        const AST::BinaryExpression *binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a logical-and binary expression.
     *
     * @param binaryExpr The AST node representing the binary expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalAnd(
        const AST::BinaryExpression *binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a logical-or binary expression.
     *
     * @param binaryExpr The AST node representing the binary expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRInstructionWithLogicalOr(
        const AST::BinaryExpression *binaryExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a copy instruction in the IR.
     *
     * @param src The source IR value.
     * @param dst The destination IR value.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRCopyInstruction(
        std::unique_ptr<IR::Value> src, std::unique_ptr<IR::Value> dst,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a jump instruction in the IR.
     *
     * @param target The target label for the jump.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRJumpInstruction(
        std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a jump-if-zero instruction in the IR.
     *
     * @param condition The condition IR value.
     * @param target The target label for the jump.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRJumpIfZeroInstruction(
        std::unique_ptr<IR::Value> condition, std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a jump-if-not-zero instruction in the IR.
     *
     * @param condition The condition IR value.
     * @param target The target label for the jump.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRJumpIfNotZeroInstruction(
        std::unique_ptr<IR::Value> condition, std::string_view target,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a label instruction in the IR.
     *
     * @param identifier The label identifier.
     * @param instructions The vector to store the generated IR instructions.
     */
    void generateIRLabelInstruction(
        std::string_view identifier,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a function call instruction in the IR.
     *
     * @param functionIdentifier The identifier of the function to call.
     * @param args The arguments for the function call.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value representing the return value
     * of the function call.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRFunctionCallInstruction(
        std::string_view functionIdentifier,
        std::unique_ptr<std::vector<std::unique_ptr<IR::Value>>> args,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate the IR for a cast expression.
     *
     * @param castExpr The AST node representing the cast expression.
     * @param instructions The vector to store the generated IR instructions.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue> generateIRCastInstruction(
        const AST::CastExpression *castExpr,
        std::vector<std::unique_ptr<IR::Instruction>> &instructions);

    /**
     * Generate a temporary variable name.
     *
     * @return The generated temporary variable name.
     */
    [[nodiscard]] std::string generateIRTemporary();

    /**
     * Generate a unique label for false branches.
     *
     * @return The generated false label.
     */
    [[nodiscard]] std::string generateIRFalseLabel();

    /**
     * Generate a unique label for true branches.
     *
     * @return The generated true label.
     */
    [[nodiscard]] std::string generateIRTrueLabel();

    /**
     * Generate a unique label for result branches.
     *
     * @return The generated result label.
     */
    [[nodiscard]] std::string generateIRResultLabel();

    /**
     * Generate a unique label for the end of a control flow structure.
     *
     * @return The generated end label.
     */
    [[nodiscard]] std::string generateIREndLabel();

    /**
     * Generate a unique label for else branches.
     *
     * @return The generated else label.
     */
    [[nodiscard]] std::string generateIRElseLabel();

    /**
     * Generate a unique label for e2 branches.
     *
     * @return The generated e2 label.
     */
    [[nodiscard]] std::string generateIRE2Label();

    /**
     * Generate a unique label for continue statements in loops.
     *
     * @param loopLabelingLabel The base label for the loop.
     * @return The generated continue loop label.
     */
    [[nodiscard]] std::string
    generateIRContinueLoopLabel(std::string_view loopLabelingLabel) const;

    /**
     * Generate a unique label for break statements in loops.
     *
     * @param loopLabelingLabel The base label for the loop.
     * @return The generated break loop label.
     */
    [[nodiscard]] std::string
    generateIRBreakLoopLabel(std::string_view loopLabelingLabel) const;

    /**
     * Generate a unique label for start of control flow structures.
     *
     * @return The generated start label.
     */
    [[nodiscard]] std::string generateIRStartLabel() const;

    /**
     * Convert the frontend symbol table to IR static variables.
     *
     * @return A unique pointer to a vector of IR static variables.
     */
    [[nodiscard]] std::unique_ptr<
        std::vector<std::unique_ptr<IR::StaticVariable>>>
    convertFrontendSymbolTableToIRStaticVariables();

    /**
     * Convert the unary operator in the unary expression to a IR unary
     * operator.
     *
     * @param op The unary operator in the AST.
     * @return The corresponding IR unary operator.
     */
    [[nodiscard]] std::unique_ptr<IR::UnaryOperator>
    convertUnop(const AST::UnaryOperator *op);

    /**
     * Convert the binary operator in the binary expression to a IR binary
     * operator.
     *
     * @param op The binary operator in the AST.
     * @return The corresponding IR binary operator.
     */
    [[nodiscard]] std::unique_ptr<IR::BinaryOperator>
    convertBinop(const AST::BinaryOperator *op);

    /**
     * Generate an IR variable for the result of a binary expression.
     *
     * @param binaryExpr The AST node representing the binary expression.
     * @return The generated IR variable value.
     */
    [[nodiscard]] std::unique_ptr<IR::VariableValue>
    generateIRVariable(const AST::BinaryExpression *binaryExpr);
};
} // namespace IR

#endif // MIDEND_IR_GENERATOR_H
