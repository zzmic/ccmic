#ifndef PRINT_VISITOR_H
#define PRINT_VISITOR_H

#include "visitor.h"
#include <iostream>

namespace AST {
class PrintVisitor : public Visitor {
  public:
    void visit(Program &program) override;
    void visit(Function &function) override;
    void visit(Block &block) override;
    void visit(SBlockItem &sBlockItem) override;
    void visit(DBlockItem &dBlockItem) override;
    void visit(Declaration &declaration) override;
    void visit(ReturnStatement &returnStatement) override;
    void visit(ExpressionStatement &expressionStatement) override;
    void visit(IfStatement &ifStatement) override;
    void visit(CompoundStatement &compoundStatement) override;
    void visit(NullStatement &nullStatement) override;
    void visit(ConstantExpression &constantExpression) override;
    void visit(VariableExpression &variableExpression) override;
    void visit(UnaryExpression &unaryExpression) override;
    void visit(BinaryExpression &binaryExpression) override;
    void visit(AssignmentExpression &assignmentExpression) override;
    void visit(ConditionalExpression &conditionalExpression) override;
    void visit(ComplementOperator &complementOperator) override;
    void visit(NegateOperator &negateOperator) override;
    void visit(NotOperator &notOperator) override;
    void visit(AddOperator &addOperator) override;
    void visit(SubtractOperator &subtractOperator) override;
    void visit(MultiplyOperator &multiplyOperator) override;
    void visit(DivideOperator &divideOperator) override;
    void visit(RemainderOperator &remainderOperator) override;
    void visit(AndOperator &andOperator) override;
    void visit(OrOperator &orOperator) override;
    void visit(EqualOperator &equalOperator) override;
    void visit(NotEqualOperator &notEqualOperator) override;
    void visit(LessThanOperator &lessThanOperator) override;
    void visit(LessThanOrEqualOperator &lessThanOrEqualOperator) override;
    void visit(GreaterThanOperator &greaterThanOperator) override;
    void visit(GreaterThanOrEqualOperator &greaterThanOrEqualOperator) override;
    void visit(AssignmentOperator &assignmentOperator) override;
};
} // namespace AST

#endif // PRINT_VISITOR_H
