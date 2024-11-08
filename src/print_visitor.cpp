/*
 * This file defines the implementation of the pretty printer for the AST and
 * concretizes the abstract visitor interface.
 */
#include "print_visitor.h"
#include "expression.h"
#include "function.h"
#include "program.h"
#include "statement.h"

namespace AST {
void PrintVisitor::visit(Program &program) {
    std::cout << "Program(\n";
    program.getFunction()->accept(*this);
    std::cout << "\n)\n";
}

void PrintVisitor::visit(Function &function) {
    std::cout << "Function(\n";
    std::cout << "name=\"" << function.getName() << "\",\n";
    std::cout << "body=";
    function.getBody()->accept(*this);
    std::cout << "\n)";
}

void PrintVisitor::visit(ReturnStatement &returnStatement) {
    std::cout << "Return(\n";
    returnStatement.getExpression()->accept(*this);
    std::cout << "\n)";
}

void PrintVisitor::visit(ConstantExpression &constantExpression) {
    std::cout << "Constant(" << constantExpression.getValue() << ")";
}

void PrintVisitor::visit(UnaryExpression &unaryExpression) {
    std::cout << "UnaryExpression(\n";
    unaryExpression.getOperator()->accept(*this);
    std::cout << "\n";
    unaryExpression.getExpression()->accept(*this);
    std::cout << "\n)";
}

void PrintVisitor::visit(BinaryExpression &binaryExpression) {
    std::cout << "BinaryExpression(\n";
    binaryExpression.getLeft()->accept(*this);
    std::cout << " " << binaryExpression.getOperator() << " ";
    binaryExpression.getRight()->accept(*this);
    std::cout << "\n";
}

void PrintVisitor::visit(ComplementOperator &complementOperator) {
    std::cout << "ComplementOperator(" << complementOperator.opInString()
              << ")";
}

void PrintVisitor::visit(NegateOperator &negateOperator) {
    std::cout << "NegateOperator(" << negateOperator.opInString() << ")";
}

} // Namespace AST
