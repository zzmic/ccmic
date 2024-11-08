

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
class ReturnStatement;
class ConstantExpression;
class UnaryExpression;
class BinaryExpression;
class ComplementOperator;
class NegateOperator;

class Visitor {
  public:
    virtual ~Visitor() = default;
    // Declare pure virtual functions for each AST node (`= 0`).
    // This in turn implies that the whole visitor class is an abstract class.
    virtual void visit(Program &program) = 0;
    virtual void visit(Function &function) = 0;
    virtual void visit(ReturnStatement &returnStatement) = 0;
    virtual void visit(ConstantExpression &constantExpression) = 0;
    virtual void visit(UnaryExpression &unaryExpression) = 0;
    virtual void visit(BinaryExpression &binaryExpression) = 0;
    virtual void visit(ComplementOperator &complementOperator) = 0;
    virtual void visit(NegateOperator &negateOperator) = 0;
};
} // Namespace AST

#endif // VISITOR_H
