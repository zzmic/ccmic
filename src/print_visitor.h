

#ifndef PRINT_VISITOR_H
#define PRINT_VISITOR_H

#include "visitor.h"
#include <iostream>

namespace AST {
class PrintVisitor : public Visitor {
  public:
    void visit(Program &program) override;
    void visit(Function &function) override;
    void visit(ReturnStatement &returnStatement) override;
    void visit(ConstantExpression &constantExpression) override;
    void visit(UnaryExpression &unaryExpression) override;
    void visit(BinaryExpression &binaryExpression) override;
    void visit(ComplementOperator &complementOperator) override;
    void visit(NegateOperator &negateOperator) override;
};
} // namespace AST

#endif // PRINT_VISITOR_H
