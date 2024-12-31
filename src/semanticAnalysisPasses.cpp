#include "semanticAnalysisPasses.h"
#include <sstream>

namespace AST {
int VariableResolutionPass::resolveVariables(std::shared_ptr<Program> program) {
    auto function = program->getFunction();
    // Initialize an empty variable map (that will be passed to the helpers).
    // Instead of maintaining a "global" variable map, we pass the variable map
    // to the helper functions to, together with `copyVariableMap`, ensure that
    // each variable map is corresponding to a specific block.
    auto variableMap = std::unordered_map<std::string, MapEntry>();

    // Resolve the variables in the function block.
    auto resolvedBlock = resolveBlock(function->getBody(), variableMap);
    function->setBody(resolvedBlock);

    return this->variableResolutionCounter;
}

std::unordered_map<std::string, MapEntry>
VariableResolutionPass::copyVariableMap(
    std::unordered_map<std::string, MapEntry> &variableMap) {
    // Copy the variable map with the fromCurrentBlock flag set to false for
    // every entry.
    auto copiedVariableMap = std::unordered_map<std::string, MapEntry>();
    for (auto &entry : variableMap) {
        copiedVariableMap[entry.first] =
            MapEntry(entry.second.getNewName(), false);
    }
    return copiedVariableMap;
}

std::string VariableResolutionPass::generateUniqueVariableName(
    const std::string &identifier) {
    // Return the string representation of the (unique) variable name.
    return identifier + "." + std::to_string(this->variableResolutionCounter++);
}

std::shared_ptr<Declaration> VariableResolutionPass::resolveVariableDeclaration(
    std::shared_ptr<Declaration> declaration,
    std::unordered_map<std::string, MapEntry> &variableMap) {
    // Get the identifier from the declaration, check if it is already in the
    // variable map, and generate a unique variable name for it if it is not.
    auto declarationIdentifier = declaration->getIdentifier();
    if (variableMap.find(declarationIdentifier) != variableMap.end() &&
        variableMap[declarationIdentifier].isFromCurrentBlock()) {
        std::stringstream msg;
        msg << "Duplicate variable declaration: " << declarationIdentifier;
        throw std::runtime_error(msg.str());
    }
    auto uniqueVariableName = generateUniqueVariableName(declarationIdentifier);
    variableMap[declarationIdentifier] = MapEntry(uniqueVariableName, true);

    // If the declaration has an initializer, resolve the expression in the
    // initializer.
    auto optInitializer = declaration->getOptInitializer();
    if (optInitializer.has_value()) {
        auto tmpInitializer =
            resolveExpression(optInitializer.value(), variableMap);
        optInitializer = std::make_optional(tmpInitializer);
    }

    // Return a new declaration with the resolved identifier and initializer.
    return std::make_shared<Declaration>(
        variableMap[declarationIdentifier].getNewName(), optInitializer);
}

std::shared_ptr<Statement> VariableResolutionPass::resolveStatement(
    std::shared_ptr<Statement> statement,
    std::unordered_map<std::string, MapEntry> &variableMap) {
    if (auto returnStatement =
            std::dynamic_pointer_cast<ReturnStatement>(statement)) {
        // If the statement is a return statement, resolve the expression in the
        // return statement.
        auto resolvedExpression =
            resolveExpression(returnStatement->getExpression(), variableMap);
        // Return a new return statement with the resolved expression.
        return std::make_shared<ReturnStatement>(resolvedExpression);
    }
    else if (auto expressionStatement =
                 std::dynamic_pointer_cast<ExpressionStatement>(statement)) {
        // If the statement is an expression statement, resolve the expression
        // in the expression statement.
        auto resolvedExpression = resolveExpression(
            expressionStatement->getExpression(), variableMap);
        // Return a new expression statement with the resolved expression.
        return std::make_shared<ExpressionStatement>(resolvedExpression);
    }
    else if (auto compoundStatement =
                 std::dynamic_pointer_cast<CompoundStatement>(statement)) {
        // Copy the variable map (with modifications) and resolve the block in
        // the compound statement.
        auto copiedVariableMap = copyVariableMap(variableMap);
        auto resolvedBlock =
            resolveBlock(compoundStatement->getBlock(), copiedVariableMap);
        return std::make_shared<CompoundStatement>(resolvedBlock);
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        // If the statement is an if-statement, resolve the condition
        // expression, then-statement, and (optional) else-statement in the
        // if-statement.
        auto resolvedCondition =
            resolveExpression(ifStatement->getCondition(), variableMap);
        auto resolvedThenStatement =
            resolveStatement(ifStatement->getThenStatement(), variableMap);
        if (ifStatement->getElseOptStatement().has_value()) {
            auto resolvedElseStatement = resolveStatement(
                ifStatement->getElseOptStatement().value(), variableMap);
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
    std::shared_ptr<Expression> expression,
    std::unordered_map<std::string, MapEntry> &variableMap) {
    if (auto assignmentExpression =
            std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        if (!(std::dynamic_pointer_cast<VariableExpression>(
                assignmentExpression->getLeft()))) {
            throw std::runtime_error("Invalid lvalue in assignment expression");
        }
        auto resolvedLeft =
            resolveExpression(assignmentExpression->getLeft(), variableMap);
        auto resolvedRight =
            resolveExpression(assignmentExpression->getRight(), variableMap);
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
        if (variableMap.find(identifier) == variableMap.end()) {
            std::stringstream msg;
            msg << "Undeclared variable: " << identifier;
            throw std::runtime_error(msg.str());
        }
        return std::make_shared<VariableExpression>(
            variableMap[identifier].getNewName());
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
            resolveExpression(unaryExpression->getExpression(), variableMap);

        // Return a new unary expression with the resolved expression.
        return std::make_shared<UnaryExpression>(
            unaryExpression->getOperator(),
            std::static_pointer_cast<Factor>(resolvedExpression));
    }
    else if (auto binaryExpression =
                 std::dynamic_pointer_cast<BinaryExpression>(expression)) {
        // If the expression is a binary expression, resolve the left and right
        // expressions in the binary expression.
        auto resolvedLeft =
            resolveExpression(binaryExpression->getLeft(), variableMap);
        auto resolvedRight =
            resolveExpression(binaryExpression->getRight(), variableMap);

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
        auto resolvedCondition = resolveExpression(
            conditionalExpression->getCondition(), variableMap);
        auto resolvedThenExpression = resolveExpression(
            conditionalExpression->getThenExpression(), variableMap);
        auto resolvedElseExpression = resolveExpression(
            conditionalExpression->getElseExpression(), variableMap);

        // Return a new conditional expression with the resolved condition
        // expression, then-expression, and else-expression.
        return std::make_shared<ConditionalExpression>(
            resolvedCondition, resolvedThenExpression, resolvedElseExpression);
    }
    else {
        throw std::runtime_error("Unsupported expression type");
    }
}

std::shared_ptr<Block> VariableResolutionPass::resolveBlock(
    std::shared_ptr<Block> block,
    std::unordered_map<std::string, MapEntry> &variableMap) {
    // Get the block items from the block and resolve the variables in each
    // block item.
    auto blockItems = block->getBlockItems();
    for (auto &blockItem : *blockItems) {
        if (auto dBlockItem =
                std::dynamic_pointer_cast<DBlockItem>(blockItem)) {
            auto resolvedDeclaration = resolveVariableDeclaration(
                dBlockItem->getDeclaration(), variableMap);
            dBlockItem->setDeclaration(resolvedDeclaration);
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            auto resolvedStatement =
                resolveStatement(sBlockItem->getStatement(), variableMap);
            sBlockItem->setStatement(resolvedStatement);
        }
        else {
            throw std::runtime_error("Unsupported block item type");
        }
    }

    // Return a new block with the resolved block items.
    return std::make_shared<Block>(blockItems);
}

} // namespace AST
