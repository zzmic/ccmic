/*
 * This file defines the implementation of the pretty printer for the AST and
 * concretizes the abstract visitor interface.
 */
#include "printVisitor.h"
#include "blockItem.h"
#include "constant.h"
#include "declaration.h"
#include "expression.h"
#include "forInit.h"
#include "function.h"
#include "operator.h"
#include "program.h"
#include "statement.h"
#include "storageClass.h"
#include "type.h"
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace AST {
void PrintVisitor::visit(Program &program) {
    std::cout << "Program(\n";

    auto &declarations = program.getDeclarations();
    for (auto it = declarations.begin(); it != declarations.end(); ++it) {
        auto &functionDeclaration = *it;
        functionDeclaration->accept(*this);
        const bool isLast = (std::next(it) == declarations.end());
        if (!isLast) {
            std::cout << ",\n";
        }
        else {
            std::cout << "\n";
        }
    }

    std::cout << ")\n";
}

void PrintVisitor::visit(Function &function) {
    std::cout << "Function(\n";
    std::cout << "name = \"";

    if (function.getIdentifier().size() > 0) {
        std::cout << function.getIdentifier();
    }
    else {
        throw std::logic_error("Null name in function in PrintVisitor");
    }

    std::cout << "\",\n";
    std::cout << "body = ";

    auto functionBody = function.getBody();
    if (functionBody) {
        auto &blockItems = functionBody->getBlockItems();
        for (auto &blockItem : blockItems) {
            blockItem->accept(*this);
        }
    }
    else {
        throw std::logic_error("Null body in function in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(Block &block) {
    std::cout << "Block(";

    for (auto &blockItem : block.getBlockItems()) {
        blockItem->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(SBlockItem &sBlockItem) {
    std::cout << "\nSBlockItem(\n";

    if (sBlockItem.getStatement()) {
        sBlockItem.getStatement()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null statement in statement block item in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(DBlockItem &dBlockItem) {
    std::cout << "\nDBlockItem(\n";

    if (dBlockItem.getDeclaration()) {
        dBlockItem.getDeclaration()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null declaration in declaration block item in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(VariableDeclaration &declaration) {
    std::cout << "VariableDeclaration(\n";

    std::cout << "identifier = ";

    if (declaration.getIdentifier().size() > 0) {
        std::cout << declaration.getIdentifier();
    }
    else {
        throw std::logic_error(
            "Null identifier in declaration in PrintVisitor");
    }

    if (declaration.getOptInitializer()) {
        std::cout << "\ninitializer = ";
        declaration.getOptInitializer()->accept(*this);
    }

    if (declaration.getVarType()) {
        std::cout << "\ntype = ";
        declaration.getVarType()->accept(*this);
    }
    else {
        throw std::logic_error("Null type in declaration in PrintVisitor");
    }

    if (declaration.getOptStorageClass()) {
        std::cout << "\nstorageClass = ";
        declaration.getOptStorageClass()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(FunctionDeclaration &functionDeclaration) {
    std::cout << "FunctionDeclaration(\n";

    std::cout << "identifier = ";

    if (functionDeclaration.getIdentifier().size() > 0) {
        std::cout << functionDeclaration.getIdentifier();
    }
    else {
        throw std::logic_error(
            "Null identifier in function declaration in PrintVisitor");
    }

    std::cout << "\nparameters = (";

    auto &parameters = functionDeclaration.getParameterIdentifiers();
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        auto &parameter = *it;
        std::cout << parameter;
        if (std::next(it) != parameters.end()) {
            std::cout << ", ";
        }
    }

    std::cout << ")";

    if (functionDeclaration.getOptBody()) {
        std::cout << "\nbody = ";
        functionDeclaration.getOptBody()->accept(*this);
    }

    std::cout << "\nfuntionType = ";
    if (functionDeclaration.getFunType()) {
        functionDeclaration.getFunType()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null function type in function declaration in PrintVisitor");
    }

    if (functionDeclaration.getOptStorageClass()) {
        std::cout << "\nstorageClass = ";
        functionDeclaration.getOptStorageClass()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(IntType &intType) {
    (void)intType;
    std::cout << "IntType()";
}

void PrintVisitor::visit(LongType &longType) {
    (void)longType;
    std::cout << "LongType()";
}

void PrintVisitor::visit(UIntType &uintType) {
    (void)uintType;
    std::cout << "UIntType()";
}

void PrintVisitor::visit(ULongType &ulongType) {
    (void)ulongType;
    std::cout << "ULongType()";
}

void PrintVisitor::visit(FunctionType &functionType) {
    std::cout << "FunctionType(\n";

    std::cout << "parameters = (";

    auto &parameters = functionType.getParameterTypes();
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        auto &parameter = *it;
        parameter->accept(*this);
        if (std::next(it) != parameters.end()) {
            std::cout << ", ";
        }
    }

    std::cout << ")";

    std::cout << "\nreturnType = ";
    const_cast<Type &>(functionType.getReturnType()).accept(*this);

    std::cout << "\n)";
}

void PrintVisitor::visit(StaticStorageClass &staticStorageClass) {
    (void)staticStorageClass;
    std::cout << "StaticStorageClass()";
}

void PrintVisitor::visit(ExternStorageClass &externStorageClass) {
    (void)externStorageClass;
    std::cout << "ExternStorageClass()";
}

void PrintVisitor::visit(InitDecl &initDecl) {
    std::cout << "InitDecl(\n";

    if (initDecl.getVariableDeclaration()) {
        initDecl.getVariableDeclaration()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null declaration in init declaration in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(InitExpr &initExpr) {
    std::cout << "InitExpr(\n";

    if (initExpr.getExpression()) {
        initExpr.getExpression()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(ReturnStatement &returnStatement) {
    std::cout << "Return(";

    if (returnStatement.getExpression()) {
        returnStatement.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null expression in return statement in PrintVisitor");
    }

    std::cout << ")";
}

void PrintVisitor::visit(ExpressionStatement &expressionStatement) {
    std::cout << "ExpressionStatement(\n";

    if (expressionStatement.getExpression()) {
        expressionStatement.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null expression in expression statement in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(IfStatement &ifStatement) {
    std::cout << "IfStatement(\n";

    std::cout << "condition = ";

    if (ifStatement.getCondition()) {
        ifStatement.getCondition()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null condition in if-statement in PrintVisitor");
    }

    std::cout << "\nthen = ";

    if (ifStatement.getThenStatement()) {
        ifStatement.getThenStatement()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null then-statement in if-statement in PrintVisitor");
    }

    if (ifStatement.getElseOptStatement()) {
        std::cout << "\nelse = ";
        ifStatement.getElseOptStatement()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(CompoundStatement &compoundStatement) {
    std::cout << "CompoundStatement(\n";

    if (compoundStatement.getBlock()) {
        compoundStatement.getBlock()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null block in compound statement in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(BreakStatement &breakStatement) {
    std::cout << "BreakStatement()" << "_" << breakStatement.getLabel();
}

void PrintVisitor::visit(ContinueStatement &continueStatement) {
    (void)continueStatement;
    std::cout << "ContinueStatement()" << "_" << continueStatement.getLabel();
}

void PrintVisitor::visit(WhileStatement &whileStatement) {
    std::cout << "WhileStatement" << "_" << whileStatement.getLabel() << "(\n";

    std::cout << "condition = ";

    if (whileStatement.getCondition()) {
        whileStatement.getCondition()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null condition in while-statement in PrintVisitor");
    }

    std::cout << "\nbody = ";

    if (whileStatement.getBody()) {
        whileStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error("Null body in while-statement in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(DoWhileStatement &doWhileStatement) {
    std::cout << "DoWhileStatement" << "_" << doWhileStatement.getLabel()
              << "(\n";

    std::cout << "condition = ";

    if (doWhileStatement.getCondition()) {
        doWhileStatement.getCondition()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null condition in do-while-statement in PrintVisitor");
    }

    std::cout << "\nbody = ";

    if (doWhileStatement.getBody()) {
        doWhileStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null body in do-while-statement in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(ForStatement &forStatement) {
    std::cout << "ForStatement" << "_" << forStatement.getLabel() << "(\n";

    std::cout << "init = ";

    if (forStatement.getForInit()) {
        forStatement.getForInit()->accept(*this);
    }
    else {
        throw std::logic_error("Null init in for-statement in PrintVisitor");
    }

    if (forStatement.getOptCondition()) {
        std::cout << "\ncondition = ";
        forStatement.getOptCondition()->accept(*this);
    }

    if (forStatement.getOptPost()) {
        std::cout << "\npost = ";
        forStatement.getOptPost()->accept(*this);
    }

    std::cout << "\nbody = ";

    if (forStatement.getBody()) {
        forStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error("Null body in for-statement in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(NullStatement &nullStatement) {
    (void)nullStatement;
    std::cout << "NullStatement()";
}

void PrintVisitor::visit(ConstantExpression &constantExpression) {
    std::cout << "ConstantExpression(";

    auto constant = constantExpression.getConstant();
    if (auto *intConst = dynamic_cast<ConstantInt *>(constant)) {
        std::cout << intConst->getValue();
    }
    else if (auto *longConst = dynamic_cast<ConstantLong *>(constant)) {
        std::cout << longConst->getValue();
    }
    else if (auto *uintConst = dynamic_cast<ConstantUInt *>(constant)) {
        std::cout << uintConst->getValue();
    }
    else if (auto *ulongConst = dynamic_cast<ConstantULong *>(constant)) {
        std::cout << ulongConst->getValue();
    }
    else {
        throw std::logic_error(
            "Unsupported constant type in constant expression in PrintVisitor");
    }

    std::cout << ")";
}

void PrintVisitor::visit(VariableExpression &variableExpression) {
    std::cout << "VariableExpression(";

    if (variableExpression.getIdentifier().size() > 0) {
        std::cout << variableExpression.getIdentifier();
    }
    else {
        throw std::logic_error(
            "Null identifier in variable expression in PrintVisitor");
    }

    std::cout << ")";
}

void PrintVisitor::visit(CastExpression &castExpression) {
    std::cout << "CastExpression(\n";

    std::cout << "targetType = ";

    if (castExpression.getTargetType()) {
        castExpression.getTargetType()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null target type in cast expression in PrintVisitor");
    }

    std::cout << "\nexpression = ";

    if (castExpression.getExpression()) {
        castExpression.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null expression in cast expression in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(UnaryExpression &unaryExpression) {
    std::cout << "UnaryExpression(\n";

    if (unaryExpression.getOperator()) {
        unaryExpression.getOperator()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null operator in unary expression in PrintVisitor");
    }

    std::cout << "\n";

    if (unaryExpression.getExpression()) {
        unaryExpression.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null expression in unary expression in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(BinaryExpression &binaryExpression) {
    std::cout << "BinaryExpression(\n";

    if (binaryExpression.getLeft()) {
        binaryExpression.getLeft()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null left operand in binary expression in PrintVisitor");
    }

    std::cout << "\n";

    if (binaryExpression.getOperator()) {
        binaryExpression.getOperator()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null operator in binary expression in PrintVisitor");
    }

    std::cout << "\n";

    if (binaryExpression.getRight()) {
        binaryExpression.getRight()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null right operand in binary expression in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(AssignmentExpression &assignmentExpression) {
    std::cout << "AssignmentExpression(\n";

    if (assignmentExpression.getLeft()) {
        assignmentExpression.getLeft()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null left operand in assignment expression in PrintVisitor");
    }

    std::cout << "\n";

    if (assignmentExpression.getRight()) {
        assignmentExpression.getRight()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null right operand in assignment expression in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(ConditionalExpression &conditionalExpression) {
    std::cout << "ConditionalExpression(\n";

    std::cout << "condition = ";

    if (conditionalExpression.getCondition()) {
        conditionalExpression.getCondition()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null condition in conditional expression in PrintVisitor");
    }

    std::cout << "\ntrue = ";

    if (conditionalExpression.getThenExpression()) {
        conditionalExpression.getThenExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null true expression in conditional expression in PrintVisitor");
    }

    std::cout << "\nfalse = ";

    if (conditionalExpression.getElseExpression()) {
        conditionalExpression.getElseExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null false expression in conditional expression in PrintVisitor");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(FunctionCallExpression &functionCallExpression) {
    std::cout << "FunctionCallExpression(\n";

    std::cout << "function = ";

    if (functionCallExpression.getIdentifier().size() > 0) {
        std::cout << functionCallExpression.getIdentifier();
    }
    else {
        throw std::logic_error(
            "Null function in function call expression in PrintVisitor");
    }

    std::cout << "\nargs = ";

    auto &args = functionCallExpression.getArguments();
    for (auto it = args.begin(); it != args.end(); ++it) {
        auto &arg = *it;
        arg->accept(*this);
        const bool isLast = (std::next(it) == args.end());
        if (!isLast) {
            std::cout << ", ";
        }
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(ComplementOperator &complementOperator) {
    std::cout << "ComplementOperator(" << complementOperator.opInString()
              << ")";
}

void PrintVisitor::visit(NegateOperator &negateOperator) {
    std::cout << "NegateOperator(" << negateOperator.opInString() << ")";
}

void PrintVisitor::visit(NotOperator &notOperator) {
    std::cout << "NotOperator(" << notOperator.opInString() << ")";
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

void PrintVisitor::visit(AndOperator &andOperator) {
    std::cout << "AndOperator(" << andOperator.opInString() << ")";
}

void PrintVisitor::visit(OrOperator &orOperator) {
    std::cout << "OrOperator(" << orOperator.opInString() << ")";
}

void PrintVisitor::visit(EqualOperator &equalOperator) {
    std::cout << "EqualOperator(" << equalOperator.opInString() << ")";
}

void PrintVisitor::visit(NotEqualOperator &notEqualOperator) {
    std::cout << "NotEqualOperator(" << notEqualOperator.opInString() << ")";
}

void PrintVisitor::visit(LessThanOperator &lessThanOperator) {
    std::cout << "LessThanOperator(" << lessThanOperator.opInString() << ")";
}

void PrintVisitor::visit(LessThanOrEqualOperator &lessThanOrEqualOperator) {
    std::cout << "LessThanOrEqualOperator("
              << lessThanOrEqualOperator.opInString() << ")";
}

void PrintVisitor::visit(GreaterThanOperator &greaterThanOperator) {
    std::cout << "GreaterThanOperator(" << greaterThanOperator.opInString()
              << ")";
}

void PrintVisitor::visit(
    GreaterThanOrEqualOperator &greaterThanOrEqualOperator) {
    std::cout << "GreaterThanOrEqualOperator("
              << greaterThanOrEqualOperator.opInString() << ")";
}

void PrintVisitor::visit(AssignmentOperator &assignmentOperator) {
    std::cout << "AssignmentOperator(" << assignmentOperator.opInString()
              << ")";
}

void PrintVisitor::visit(ConstantInt &constantInt) {
    std::cout << "ConstantInt(" << constantInt.getValue() << ")";
}

void PrintVisitor::visit(ConstantLong &constantLong) {
    std::cout << "ConstantLong(" << constantLong.getValue() << ")";
}

void PrintVisitor::visit(ConstantUInt &constantUInt) {
    std::cout << "ConstantUInt(" << constantUInt.getValue() << ")";
}

void PrintVisitor::visit(ConstantULong &constantULong) {
    std::cout << "ConstantULong(" << constantULong.getValue() << ")";
}
} // Namespace AST
