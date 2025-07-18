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

    if (program.getDeclarations()) {
        auto &declarations = *program.getDeclarations();
        for (auto it = declarations.begin(); it != declarations.end(); it++) {
            auto &functionDeclaration = *it;
            functionDeclaration->accept(*this);
            bool isLast = (std::next(it) == declarations.end());
            if (!isLast) {
                std::cout << ",\n";
            }
            else {
                std::cout << "\n";
            }
        }
    }
    else {
        throw std::logic_error("Null declarations in program");
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
        throw std::logic_error("Null name in function");
    }

    std::cout << "\",\n";
    std::cout << "body = ";

    auto functionBody = function.getBody();
    if (functionBody) {
        auto blockItems = functionBody->getBlockItems();
        for (auto &blockItem : *blockItems) {
            blockItem->accept(*this);
        }
    }
    else {
        throw std::logic_error("Null body in function");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(Block &block) {
    std::cout << "Block(";

    for (auto &blockItem : *block.getBlockItems()) {
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
        throw std::logic_error("Null statement in statement block item");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(DBlockItem &dBlockItem) {
    std::cout << "\nDBlockItem(\n";

    if (dBlockItem.getDeclaration()) {
        dBlockItem.getDeclaration()->accept(*this);
    }
    else {
        throw std::logic_error("Null declaration in declaration block item");
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
        throw std::logic_error("Null identifier in declaration");
    }

    if (declaration.getOptInitializer().has_value()) {
        std::cout << "\ninitializer = ";
        declaration.getOptInitializer().value()->accept(*this);
    }

    if (declaration.getVarType()) {
        std::cout << "\ntype = ";
        declaration.getVarType()->accept(*this);
    }
    else {
        throw std::logic_error("Null type in declaration");
    }

    if (declaration.getOptStorageClass().has_value()) {
        std::cout << "\nstorageClass = ";
        declaration.getOptStorageClass().value()->accept(*this);
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
        throw std::logic_error("Null identifier in function declaration");
    }

    std::cout << "\nparameters = (";

    auto &parameters = *functionDeclaration.getParameterIdentifiers();
    for (auto it = parameters.begin(); it != parameters.end(); it++) {
        auto &parameter = *it;
        std::cout << parameter;
        if (std::next(it) != parameters.end()) {
            std::cout << ", ";
        }
    }

    std::cout << ")";

    if (functionDeclaration.getOptBody().has_value()) {
        std::cout << "\nbody = ";
        functionDeclaration.getOptBody().value()->accept(*this);
    }

    std::cout << "\nfuntionType = ";
    if (functionDeclaration.getFunType()) {
        functionDeclaration.getFunType()->accept(*this);
    }
    else {
        throw std::logic_error("Null function type in function declaration");
    }

    if (functionDeclaration.getOptStorageClass().has_value()) {
        std::cout << "\nstorageClass = ";
        functionDeclaration.getOptStorageClass().value()->accept(*this);
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

void PrintVisitor::visit(FunctionType &functionType) {
    std::cout << "FunctionType(\n";

    std::cout << "parameters = (";

    auto &parameters = *functionType.getParameterTypes();
    for (auto it = parameters.begin(); it != parameters.end(); it++) {
        auto &parameter = *it;
        parameter->accept(*this);
        if (std::next(it) != parameters.end()) {
            std::cout << ", ";
        }
    }

    std::cout << ")";

    std::cout << "\nreturnType = ";
    functionType.getReturnType()->accept(*this);

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
        throw std::logic_error("Null declaration in init declaration");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(InitExpr &initExpr) {
    std::cout << "InitExpr(\n";

    if (initExpr.getExpression().has_value()) {
        initExpr.getExpression().value()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(ReturnStatement &returnStatement) {
    std::cout << "Return(";

    if (returnStatement.getExpression()) {
        returnStatement.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error("Null expression in return statement");
    }

    std::cout << ")";
}

void PrintVisitor::visit(ExpressionStatement &expressionStatement) {
    std::cout << "ExpressionStatement(\n";

    if (expressionStatement.getExpression()) {
        expressionStatement.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error("Null expression in expression statement");
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
        throw std::logic_error("Null condition in if-statement");
    }

    std::cout << "\nthen = ";

    if (ifStatement.getThenStatement()) {
        ifStatement.getThenStatement()->accept(*this);
    }
    else {
        throw std::logic_error("Null then-statement in if-statement");
    }

    if (ifStatement.getElseOptStatement().has_value()) {
        std::cout << "\nelse = ";
        ifStatement.getElseOptStatement().value()->accept(*this);
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(CompoundStatement &compoundStatement) {
    std::cout << "CompoundStatement(\n";

    if (compoundStatement.getBlock()) {
        compoundStatement.getBlock()->accept(*this);
    }
    else {
        throw std::logic_error("Null block in compound statement");
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
        throw std::logic_error("Null condition in while-statement");
    }

    std::cout << "\nbody = ";

    if (whileStatement.getBody()) {
        whileStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error("Null body in while-statement");
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
        throw std::logic_error("Null condition in do-while-statement");
    }

    std::cout << "\nbody = ";

    if (doWhileStatement.getBody()) {
        doWhileStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error("Null body in do-while-statement");
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
        throw std::logic_error("Null init in for-statement");
    }

    if (forStatement.getOptCondition().has_value()) {
        std::cout << "\ncondition = ";
        if (forStatement.getOptCondition().value()) {
            forStatement.getOptCondition().value()->accept(*this);
        }
    }

    if (forStatement.getOptPost().has_value()) {
        std::cout << "\npost = ";
        if (forStatement.getOptPost().value()) {
            forStatement.getOptPost().value()->accept(*this);
        }
    }

    std::cout << "\nbody = ";

    if (forStatement.getBody()) {
        forStatement.getBody()->accept(*this);
    }
    else {
        throw std::logic_error("Null body in for-statement");
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
    if (auto intConst = std::dynamic_pointer_cast<ConstantInt>(constant)) {
        std::cout << intConst->getValue();
    }
    else if (auto longConst =
                 std::dynamic_pointer_cast<ConstantLong>(constant)) {
        std::cout << longConst->getValue();
    }
    else {
        throw std::logic_error("Unknown constant type in constant expression");
    }

    std::cout << ")";
}

void PrintVisitor::visit(VariableExpression &variableExpression) {
    std::cout << "VariableExpression(";

    if (variableExpression.getIdentifier().size() > 0) {
        std::cout << variableExpression.getIdentifier();
    }
    else {
        throw std::logic_error("Null identifier in variable expression");
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
        throw std::logic_error("Null target type in cast expression");
    }

    std::cout << "\nexpression = ";

    if (castExpression.getExpression()) {
        castExpression.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error("Null expression in cast expression");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(UnaryExpression &unaryExpression) {
    std::cout << "UnaryExpression(\n";

    if (unaryExpression.getOperator()) {
        unaryExpression.getOperator()->accept(*this);
    }
    else {
        throw std::logic_error("Null operator in unary expression");
    }

    std::cout << "\n";

    if (unaryExpression.getExpression()) {
        unaryExpression.getExpression()->accept(*this);
    }
    else {
        throw std::logic_error("Null expression in unary expression");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(BinaryExpression &binaryExpression) {
    std::cout << "BinaryExpression(\n";

    if (binaryExpression.getLeft()) {
        binaryExpression.getLeft()->accept(*this);
    }
    else {
        throw std::logic_error("Null left operand in binary expression");
    }

    std::cout << "\n";

    if (binaryExpression.getOperator()) {
        binaryExpression.getOperator()->accept(*this);
    }
    else {
        throw std::logic_error("Null operator in binary expression");
    }

    std::cout << "\n";

    if (binaryExpression.getRight()) {
        binaryExpression.getRight()->accept(*this);
    }
    else {
        throw std::logic_error("Null right operand in binary expression");
    }

    std::cout << "\n)";
}

void PrintVisitor::visit(AssignmentExpression &assignmentExpression) {
    std::cout << "AssignmentExpression(\n";

    if (assignmentExpression.getLeft()) {
        assignmentExpression.getLeft()->accept(*this);
    }
    else {
        throw std::logic_error("Null left operand in assignment expression");
    }

    std::cout << "\n";

    if (assignmentExpression.getRight()) {
        assignmentExpression.getRight()->accept(*this);
    }
    else {
        throw std::logic_error("Null right operand in assignment expression");
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
        throw std::logic_error("Null condition in conditional expression");
    }

    std::cout << "\ntrue = ";

    if (conditionalExpression.getThenExpression()) {
        conditionalExpression.getThenExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null true expression in conditional expression");
    }

    std::cout << "\nfalse = ";

    if (conditionalExpression.getElseExpression()) {
        conditionalExpression.getElseExpression()->accept(*this);
    }
    else {
        throw std::logic_error(
            "Null false expression in conditional expression");
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
        throw std::logic_error("Null function in function call expression");
    }

    std::cout << "\nargs = ";

    auto &args = *functionCallExpression.getArguments();
    for (auto it = args.begin(); it != args.end(); it++) {
        auto &arg = *it;
        arg->accept(*this);
        bool isLast = (std::next(it) == args.end());
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
} // Namespace AST
