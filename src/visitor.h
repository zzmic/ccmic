/*
 * This header file defines the visitor (pattern) interface.
 * The visitor pattern helps to separate the algorithm from the object structure
 * on which it operates. This makes it easier to add new operations to the
 * object structure (which may get more complex) without modifying the objects
 * themselves For each AST node, its interface will declare an accept method
 * that takes a visitor as an argument.
 */
#ifndef VISITOR_H
#define VISITOR_H

namespace AST {
// Forward declarations of AST nodes/classes.
class Program;
class Function;
class Block;
class BlockItem;
class SBlockItem;
class DBlockItem;
class Declaration;
class VariableDeclaration;
class FunctionDeclaration;
class ForInit;
class InitDecl;
class InitExpr;
class Statement;
class ReturnStatement;
class ExpressionStatement;
class IfStatement;
class CompoundStatement;
class BreakStatement;
class ContinueStatement;
class WhileStatement;
class DoWhileStatement;
class ForStatement;
class NullStatement;
class Expression;
class ConstantExpression;
class VariableExpression;
class UnaryExpression;
class BinaryExpression;
class AssignmentExpression;
class ConditionalExpression;
class FunctionCallExpression;
class Operator;
class UnaryOperator;
class ComplementOperator;
class NegateOperator;
class NotOperator;
class BinaryOperator;
class AddOperator;
class SubtractOperator;
class MultiplyOperator;
class DivideOperator;
class RemainderOperator;
class AndOperator;
class OrOperator;
class EqualOperator;
class NotEqualOperator;
class LessThanOperator;
class LessThanOrEqualOperator;
class GreaterThanOperator;
class GreaterThanOrEqualOperator;
class AssignmentOperator;

class Visitor {
  public:
    virtual ~Visitor() = default;
    // Declare pure virtual functions for each AST node (`= 0`).
    // This in turn implies that the whole visitor class is an abstract class.
    virtual void visit(Program &program) = 0;
    virtual void visit(Function &function) = 0;
    virtual void visit(Block &block) = 0;
    virtual void visit(SBlockItem &sBlockItem) = 0;
    virtual void visit(DBlockItem &dBlockItem) = 0;
    virtual void visit(VariableDeclaration &declaration) = 0;
    virtual void visit(FunctionDeclaration &functionDeclaration) = 0;
    virtual void visit(InitDecl &initDecl) = 0;
    virtual void visit(InitExpr &initExpr) = 0;
    virtual void visit(ReturnStatement &returnStatement) = 0;
    virtual void visit(ExpressionStatement &expressionStatement) = 0;
    virtual void visit(IfStatement &ifStatement) = 0;
    virtual void visit(CompoundStatement &compoundStatement) = 0;
    virtual void visit(BreakStatement &breakStatement) = 0;
    virtual void visit(ContinueStatement &continueStatement) = 0;
    virtual void visit(WhileStatement &whileStatement) = 0;
    virtual void visit(DoWhileStatement &doWhileStatement) = 0;
    virtual void visit(ForStatement &forStatement) = 0;
    virtual void visit(NullStatement &nullStatement) = 0;
    virtual void visit(ConstantExpression &constantExpression) = 0;
    virtual void visit(VariableExpression &variableExpression) = 0;
    virtual void visit(UnaryExpression &unaryExpression) = 0;
    virtual void visit(BinaryExpression &binaryExpression) = 0;
    virtual void visit(AssignmentExpression &assignmentExpression) = 0;
    virtual void visit(ConditionalExpression &conditionalExpression) = 0;
    virtual void visit(FunctionCallExpression &functionCallExpression) = 0;
    virtual void visit(ComplementOperator &complementOperator) = 0;
    virtual void visit(NegateOperator &negateOperator) = 0;
    virtual void visit(NotOperator &notOperator) = 0;
    virtual void visit(AddOperator &addOperator) = 0;
    virtual void visit(SubtractOperator &subtractOperator) = 0;
    virtual void visit(MultiplyOperator &multiplyOperator) = 0;
    virtual void visit(DivideOperator &divideOperator) = 0;
    virtual void visit(RemainderOperator &remainderOperator) = 0;
    virtual void visit(AndOperator &andOperator) = 0;
    virtual void visit(OrOperator &orOperator) = 0;
    virtual void visit(EqualOperator &equalOperator) = 0;
    virtual void visit(NotEqualOperator &notEqualOperator) = 0;
    virtual void visit(LessThanOperator &lessThanOperator) = 0;
    virtual void visit(LessThanOrEqualOperator &lessThanOrEqualOperator) = 0;
    virtual void visit(GreaterThanOperator &greaterThanOperator) = 0;
    virtual void
    visit(GreaterThanOrEqualOperator &greaterThanOrEqualOperator) = 0;
    virtual void visit(AssignmentOperator &assignmentOperator) = 0;
};
} // Namespace AST

#endif // VISITOR_H
