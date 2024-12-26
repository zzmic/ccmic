/*
 * This file defines the implementation of the pretty printer for the AST and
 * concretizes the abstract visitor interface.
 */
#include "printVisitor.h"
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
    std::cout << "ConstantExpression(" << constantExpression.getValue() << ")";
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
    std::cout << "\n";
    binaryExpression.getOperator()->accept(*this);
    std::cout << "\n";
    binaryExpression.getRight()->accept(*this);
    std::cout << "\n)";
}

void PrintVisitor::visit(ComplementOperator &complementOperator) {
    std::cout << "ComplementOperator(" << complementOperator.opInString()
              << ")";
}

void PrintVisitor::visit(NegateOperator &negateOperator) {
    std::cout << "NegateOperator(" << negateOperator.opInString() << ")";
}

void PrintVisitor::visit(AddOperator &addOperator) {
    std::cout << "AddOperator(" << addOperator.opInString() << ")";
}

void PrintVisitor::visit(SubtractOperator &subtractOperator) {
    std::cout << "SubtractOperator(" << subtractOperator.opInString() << ")";
}

void PrintVisitor::visit(MultiplyOperator &multiplyOperator) {
    std::cout << "MultiplyOperator(" << multiplyOperator.opInString() << ")";
}

void PrintVisitor::visit(DivideOperator &divideOperator) {
    std::cout << "DivideOperator(" << divideOperator.opInString() << ")";
}

void PrintVisitor::visit(RemainderOperator &remainderOperator) {
    std::cout << "RemainderOperator(" << remainderOperator.opInString() << ")";
}
} // Namespace AST
