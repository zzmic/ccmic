#include "semanticAnalysisPasses.h"
#include <sstream>

namespace AST {
int VariableResolutionPass::resolveVariables(std::shared_ptr<Program> program) {
    auto function = program->getFunction();
    // For each block item in the function body, resolve the variables in the
    // block item, either a declaration or a statement.
    for (auto &blockItem : *function->getBody()) {
        if (auto dBlockItem =
                std::dynamic_pointer_cast<DBlockItem>(blockItem)) {
            auto resolvedDeclaration =
                resolveVariableDeclaration(dBlockItem->getDeclaration());
            dBlockItem->setDeclaration(resolvedDeclaration);
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            auto resolvedStatement =
                resolveStatement(sBlockItem->getStatement());
            sBlockItem->setStatement(resolvedStatement);
        }
        else {
            throw std::runtime_error("Unsupported block item type");
        }
    }
    return this->variableResolutionCounter;
}

std::string VariableResolutionPass::generateUniqueVariableName(
    const std::string &identifier) {
    // Return the string representation of the (unique) variable name.
    return identifier + "." + std::to_string(this->variableResolutionCounter++);
}

std::shared_ptr<Declaration> VariableResolutionPass::resolveVariableDeclaration(
    std::shared_ptr<Declaration> declaration) {
    // Get the identifier from the declaration, check if it is already in the
    // variable map, and generate a unique variable name for it if it is not.
    auto declarationIdentifier = declaration->getIdentifier();
    if (this->variableMap.find(declarationIdentifier) !=
        this->variableMap.end()) {
        std::stringstream msg;
        msg << "Duplicate variable declaration: " << declarationIdentifier;
        throw std::runtime_error(msg.str());
    }
    this->variableMap[declarationIdentifier] =
        generateUniqueVariableName(declarationIdentifier);

    // If the declaration has an initializer, resolve the expression in the
    // initializer.
    auto optInitializer = declaration->getOptInitializer();
    if (optInitializer.has_value()) {
        auto tmpInitializer = resolveExpression(optInitializer.value());
        optInitializer = std::make_optional(tmpInitializer);
    }

    // Return a new declaration with the resolved identifier and initializer.
    return std::make_shared<Declaration>(
        this->variableMap[declarationIdentifier], optInitializer);
}

std::shared_ptr<Statement>
VariableResolutionPass::resolveStatement(std::shared_ptr<Statement> statement) {
    if (auto returnStatement =
            std::dynamic_pointer_cast<ReturnStatement>(statement)) {
        // If the statement is a return statement, resolve the expression in the
        // return statement.
        auto resolvedExpression =
            resolveExpression(returnStatement->getExpression());

        // Return a new return statement with the resolved expression.
        return std::make_shared<ReturnStatement>(resolvedExpression);
    }
    else if (auto expressionStatement =
                 std::dynamic_pointer_cast<ExpressionStatement>(statement)) {
        // If the statement is an expression statement, resolve the expression
        // in the expression statement.
        auto resolvedExpression =
            resolveExpression(expressionStatement->getExpression());

        // Return a new expression statement with the resolved expression.
        return std::make_shared<ExpressionStatement>(resolvedExpression);
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        // If the statement is an if-statement, resolve the condition
        // expression, then-statement, and (optional) else-statement in the
        // if-statement.
        auto resolvedCondition = resolveExpression(ifStatement->getCondition());
        auto resolvedThenStatement =
            resolveStatement(ifStatement->getThenStatement());
        if (ifStatement->getElseOptStatement().has_value()) {
            auto resolvedElseStatement =
                resolveStatement(ifStatement->getElseOptStatement().value());
            return std::make_shared<IfStatement>(resolvedCondition,
                                                 resolvedThenStatement,
                                                 resolvedElseStatement);
        }
        else {
            return std::make_shared<IfStatement>(resolvedCondition,
                                                 resolvedThenStatement);
        }
    }
    else if (auto nullStatement =
                 std::dynamic_pointer_cast<NullStatement>(statement)) {
        // If the statement is a null statement, return the null statement.
        return nullStatement;
    }
    else {
        throw std::runtime_error("Unsupported statement type");
    }
}

std::shared_ptr<Expression> VariableResolutionPass::resolveExpression(
    std::shared_ptr<Expression> expression) {
    if (auto assignmentExpression =
            std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        if (!(std::dynamic_pointer_cast<VariableExpression>(
                assignmentExpression->getLeft()))) {
            throw std::runtime_error("Invalid lvalue in assignment expression");
        }
        auto resolvedLeft = resolveExpression(assignmentExpression->getLeft());
        auto resolvedRight =
            resolveExpression(assignmentExpression->getRight());
        return std::make_shared<AssignmentExpression>(resolvedLeft,
                                                      resolvedRight);
    }
    else if (auto variableExpression =
                 std::dynamic_pointer_cast<VariableExpression>(expression)) {
        // If the expression is a variable expression, check if the variable is
        // already in the variable map. If it is not, throw an error. Otherwise,
        // return a new variable expression with the resolved identifier from
        // the variable map.
        auto identifier = variableExpression->getIdentifier();
        if (this->variableMap.find(identifier) == this->variableMap.end()) {
            std::stringstream msg;
            msg << "Undeclared variable: " << identifier;
            throw std::runtime_error(msg.str());
        }
        return std::make_shared<VariableExpression>(
            this->variableMap[identifier]);
    }
    else if (auto constantExpression =
                 std::dynamic_pointer_cast<ConstantExpression>(expression)) {
        // If the expression is a constant expression, return the constant
        // expression.
        return constantExpression;
    }
    else if (auto unaryExpression =
                 std::dynamic_pointer_cast<UnaryExpression>(expression)) {
        // If the expression is a unary expression, resolve the expression in
        // the unary expression.
        auto resolvedExpression =
            resolveExpression(unaryExpression->getExpression());

        // Return a new unary expression with the resolved expression.
        return std::make_shared<UnaryExpression>(
            unaryExpression->getOperator(),
            std::static_pointer_cast<Factor>(resolvedExpression));
    }
    else if (auto binaryExpression =
                 std::dynamic_pointer_cast<BinaryExpression>(expression)) {
        // If the expression is a binary expression, resolve the left and right
        // expressions in the binary expression.
        auto resolvedLeft = resolveExpression(binaryExpression->getLeft());
        auto resolvedRight = resolveExpression(binaryExpression->getRight());

        // Return a new binary expression with the resolved left and right
        // expressions.
        return std::make_shared<BinaryExpression>(
            resolvedLeft, binaryExpression->getOperator(), resolvedRight);
    }
    else if (auto conditionalExpression =
                 std::dynamic_pointer_cast<ConditionalExpression>(expression)) {
        // If the expression is a conditional expression, resolve the condition
        // expression, then-expression, and else-expression in the conditional
        // expression.
        auto resolvedCondition =
            resolveExpression(conditionalExpression->getCondition());
        auto resolvedThenExpression =
            resolveExpression(conditionalExpression->getThenExpression());
        auto resolvedElseExpression =
            resolveExpression(conditionalExpression->getElseExpression());

        // Return a new conditional expression with the resolved condition
        // expression, then-expression, and else-expression.
        return std::make_shared<ConditionalExpression>(
            resolvedCondition, resolvedThenExpression, resolvedElseExpression);
    }
    else {
        throw std::runtime_error("Unsupported expression type");
    }
}
} // namespace AST
