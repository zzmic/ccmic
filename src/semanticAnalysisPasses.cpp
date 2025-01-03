#include "semanticAnalysisPasses.h"
#include <sstream>

namespace AST {
/*
 * Start: Functions for the identifier-resolution pass.
 */
int IdentifierResolutionPass::resolveIdentifiers(
    std::shared_ptr<Program> program) {
    // Initialize an empty identifier map (that will be passed to the helpers).
    // Instead of maintaining a "global" identifier map, we pass the identifier
    // map to the helper functions to, together with `copyIdentifierMap`, ensure
    // that each identifier map is corresponding to a specific block.
    auto identifierMap = std::unordered_map<std::string, MapEntry>();

    // At the top level, resolve the list of function declarations in the
    // program.
    auto resolvedFunctionDeclarations =
        std::make_shared<std::vector<std::shared_ptr<FunctionDeclaration>>>();
    for (auto &functionDeclaration : *program->getFunctionDeclarations()) {
        auto resolvedFunctionDeclaration =
            resolveFunctionDeclaration(functionDeclaration, identifierMap);
        resolvedFunctionDeclarations->emplace_back(resolvedFunctionDeclaration);
    }
    program->setFunctionDeclarations(resolvedFunctionDeclarations);

    return this->variableResolutionCounter;
}

std::unordered_map<std::string, MapEntry>
IdentifierResolutionPass::copyIdentifierMap(
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Make a copy of the identifier map and set the `fromCurrentBlock` flag to
    // false for every entry.
    auto copiedIdentifierMap = std::unordered_map<std::string, MapEntry>();
    for (auto &entry : identifierMap) {
        copiedIdentifierMap[entry.first] = MapEntry(
            entry.second.getNewName(), false, entry.second.hasLinkageOrNot());
    }
    return copiedIdentifierMap;
}

std::string IdentifierResolutionPass::generateUniqueVariableName(
    const std::string &identifier) {
    // Return the string representation of the (unique) variable name.
    return identifier + "." + std::to_string(this->variableResolutionCounter++);
}

std::shared_ptr<VariableDeclaration>
IdentifierResolutionPass::resolveVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Get the identifier from the declaration, check if it is already in the
    // identifier map, and generate a unique variable name for it if it is not.
    auto declarationIdentifier = declaration->getIdentifier();
    if (identifierMap.find(declarationIdentifier) != identifierMap.end() &&
        identifierMap[declarationIdentifier].fromCurrentBlockOrNot()) {
        std::stringstream msg;
        msg << "Duplicate variable declaration: " << declarationIdentifier;
        throw std::runtime_error(msg.str());
    }
    auto uniqueVariableName = generateUniqueVariableName(declarationIdentifier);
    identifierMap[declarationIdentifier] =
        MapEntry(uniqueVariableName, true, false);

    // If the declaration has an initializer, resolve the expression in the
    // initializer.
    auto optInitializer = declaration->getOptInitializer();
    if (optInitializer.has_value()) {
        auto tmpInitializer =
            resolveExpression(optInitializer.value(), identifierMap);
        optInitializer = std::make_optional(tmpInitializer);
    }

    // Return a new declaration with the resolved identifier and initializer.
    return std::make_shared<VariableDeclaration>(
        identifierMap[declarationIdentifier].getNewName(),
        std::move(optInitializer));
}

std::shared_ptr<Statement> IdentifierResolutionPass::resolveStatement(
    std::shared_ptr<Statement> statement,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto returnStatement =
            std::dynamic_pointer_cast<ReturnStatement>(statement)) {
        // If the statement is a return statement, resolve the expression in the
        // return statement.
        auto resolvedExpression =
            resolveExpression(returnStatement->getExpression(), identifierMap);
        // Return a new return statement with the resolved expression.
        return std::make_shared<ReturnStatement>(std::move(resolvedExpression));
    }
    else if (auto expressionStatement =
                 std::dynamic_pointer_cast<ExpressionStatement>(statement)) {
        // If the statement is an expression statement, resolve the expression
        // in the expression statement.
        auto resolvedExpression = resolveExpression(
            expressionStatement->getExpression(), identifierMap);
        // Return a new expression statement with the resolved expression.
        return std::make_shared<ExpressionStatement>(
            std::move(resolvedExpression));
    }
    else if (auto compoundStatement =
                 std::dynamic_pointer_cast<CompoundStatement>(statement)) {
        // Copy the identifier map (with modifications) and resolve the block in
        // the compound statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        auto resolvedBlock =
            resolveBlock(compoundStatement->getBlock(), copiedIdentifierMap);
        return std::make_shared<CompoundStatement>(std::move(resolvedBlock));
    }
    else if (auto breakStatement =
                 std::dynamic_pointer_cast<BreakStatement>(statement)) {
        // If the statement is a break statement, return the break statement.
        return breakStatement;
    }
    else if (auto continueStatement =
                 std::dynamic_pointer_cast<ContinueStatement>(statement)) {
        // If the statement is a continue statement, return the continue
        // statement.
        return continueStatement;
    }
    else if (auto whileStatement =
                 std::dynamic_pointer_cast<WhileStatement>(statement)) {
        // If the statement is a while-statement, resolve the condition
        // expression and the body statement in the while-statement.
        auto resolvedCondition =
            resolveExpression(whileStatement->getCondition(), identifierMap);
        auto resolvedBody =
            resolveStatement(whileStatement->getBody(), identifierMap);
        return std::make_shared<WhileStatement>(std::move(resolvedCondition),
                                                std::move(resolvedBody));
    }
    else if (auto doWhileStatement =
                 std::dynamic_pointer_cast<DoWhileStatement>(statement)) {
        // If the statement is a do-while-statement, resolve the condition
        // expression and the body statement in the do-while-statement.
        auto resolvedCondition =
            resolveExpression(doWhileStatement->getCondition(), identifierMap);
        auto resolvedBody =
            resolveStatement(doWhileStatement->getBody(), identifierMap);
        return std::make_shared<DoWhileStatement>(std::move(resolvedCondition),
                                                  std::move(resolvedBody));
    }
    else if (auto forStatement =
                 std::dynamic_pointer_cast<ForStatement>(statement)) {
        // Copy the identifier map (with modifications) and resolve the
        // for-init, (optional) condition, (optional) post, and body in the
        // for-statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        auto resolvedForInit =
            resolveForInit(forStatement->getForInit(), copiedIdentifierMap);
        auto resolvedOptCondition =
            std::optional<std::shared_ptr<Expression>>();
        if (forStatement->getOptCondition().has_value()) {
            auto resolvedCondition = resolveExpression(
                forStatement->getOptCondition().value(), copiedIdentifierMap);
            resolvedOptCondition = std::make_optional(resolvedCondition);
        }
        auto resolvedOptPost = std::optional<std::shared_ptr<Expression>>();
        if (forStatement->getOptPost().has_value()) {
            auto resolvedPost = resolveExpression(
                forStatement->getOptPost().value(), copiedIdentifierMap);
            resolvedOptPost = std::make_optional(resolvedPost);
        }
        auto resolvedBody =
            resolveStatement(forStatement->getBody(), copiedIdentifierMap);
        return std::make_shared<ForStatement>(
            std::move(resolvedForInit), std::move(resolvedOptCondition),
            std::move(resolvedOptPost), std::move(resolvedBody));
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        // If the statement is an if-statement, resolve the condition
        // expression, then-statement, and (optional) else-statement in the
        // if-statement.
        auto resolvedCondition =
            resolveExpression(ifStatement->getCondition(), identifierMap);
        auto resolvedThenStatement =
            resolveStatement(ifStatement->getThenStatement(), identifierMap);
        if (ifStatement->getElseOptStatement().has_value()) {
            auto resolvedElseStatement = resolveStatement(
                ifStatement->getElseOptStatement().value(), identifierMap);
            return std::make_shared<IfStatement>(
                std::move(resolvedCondition), std::move(resolvedThenStatement),
                std::move(resolvedElseStatement));
        }
        else {
            return std::make_shared<IfStatement>(
                std::move(resolvedCondition), std::move(resolvedThenStatement));
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

std::shared_ptr<Expression> IdentifierResolutionPass::resolveExpression(
    std::shared_ptr<Expression> expression,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto assignmentExpression =
            std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        if (!(std::dynamic_pointer_cast<VariableExpression>(
                assignmentExpression->getLeft()))) {
            throw std::runtime_error("Invalid lvalue in assignment expression");
        }
        auto resolvedLeft =
            resolveExpression(assignmentExpression->getLeft(), identifierMap);
        auto resolvedRight =
            resolveExpression(assignmentExpression->getRight(), identifierMap);
        return std::make_shared<AssignmentExpression>(std::move(resolvedLeft),
                                                      std::move(resolvedRight));
    }
    else if (auto variableExpression =
                 std::dynamic_pointer_cast<VariableExpression>(expression)) {
        // If the expression is a variable expression, check if the variable is
        // already in the identifier map. If it is not, throw an error.
        // Otherwise, return a new variable expression with the resolved
        // identifier from the identifier map.
        auto identifier = variableExpression->getIdentifier();
        if (identifierMap.find(identifier) == identifierMap.end()) {
            std::stringstream msg;
            msg << "Undeclared variable: " << identifier;
            throw std::runtime_error(msg.str());
        }
        return std::make_shared<VariableExpression>(
            identifierMap[identifier].getNewName());
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
            resolveExpression(unaryExpression->getExpression(), identifierMap);
        // Return a new unary expression with the resolved expression.
        return std::make_shared<UnaryExpression>(
            unaryExpression->getOperator(),
            std::static_pointer_cast<Factor>(std::move(resolvedExpression)));
    }
    else if (auto binaryExpression =
                 std::dynamic_pointer_cast<BinaryExpression>(expression)) {
        // If the expression is a binary expression, resolve the left and right
        // expressions in the binary expression.
        auto resolvedLeft =
            resolveExpression(binaryExpression->getLeft(), identifierMap);
        auto resolvedRight =
            resolveExpression(binaryExpression->getRight(), identifierMap);
        // Return a new binary expression with the resolved left and right
        // expressions.
        return std::make_shared<BinaryExpression>(
            std::move(resolvedLeft), binaryExpression->getOperator(),
            std::move(resolvedRight));
    }
    else if (auto conditionalExpression =
                 std::dynamic_pointer_cast<ConditionalExpression>(expression)) {
        // If the expression is a conditional expression, resolve the condition
        // expression, then-expression, and else-expression in the conditional
        // expression.
        auto resolvedCondition = resolveExpression(
            conditionalExpression->getCondition(), identifierMap);
        auto resolvedThenExpression = resolveExpression(
            conditionalExpression->getThenExpression(), identifierMap);
        auto resolvedElseExpression = resolveExpression(
            conditionalExpression->getElseExpression(), identifierMap);
        // Return a new conditional expression with the resolved condition
        // expression, then-expression, and else-expression.
        return std::make_shared<ConditionalExpression>(
            std::move(resolvedCondition), std::move(resolvedThenExpression),
            std::move(resolvedElseExpression));
    }
    else if (auto functionCallExpression =
                 std::dynamic_pointer_cast<FunctionCallExpression>(
                     expression)) {
        // If the expression is a function call expression, check if the
        // function is already in the identifier map. If it is not, throw an
        // error. Otherwise, return a new function call expression with the
        // resolved identifier from the identifier map and the resolved
        // arguments.
        if (identifierMap.find(functionCallExpression->getIdentifier()) ==
            identifierMap.end()) {
            std::stringstream msg;
            msg << "Undeclared function: "
                << functionCallExpression->getIdentifier();
            throw std::runtime_error(msg.str());
        }
        auto resolvedFunctionName =
            identifierMap[functionCallExpression->getIdentifier()].getNewName();
        auto resolvedArguments =
            std::make_shared<std::vector<std::shared_ptr<Expression>>>();
        for (auto &argument : *functionCallExpression->getArguments()) {
            resolvedArguments->emplace_back(
                resolveExpression(argument, identifierMap));
        }
        return std::make_shared<FunctionCallExpression>(resolvedFunctionName,
                                                        resolvedArguments);
    }
    else {
        throw std::runtime_error("Unsupported expression type");
    }
}

std::shared_ptr<Block> IdentifierResolutionPass::resolveBlock(
    std::shared_ptr<Block> block,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Get the block items from the block and resolve the variables in each
    // block item.
    auto blockItems = block->getBlockItems();
    for (auto &blockItem : *blockItems) {
        if (auto dBlockItem =
                std::dynamic_pointer_cast<DBlockItem>(blockItem)) {
            if (auto variableDeclaration =
                    std::dynamic_pointer_cast<VariableDeclaration>(
                        dBlockItem->getDeclaration())) {
                auto resolvedDeclaration = resolveVariableDeclaration(
                    variableDeclaration, identifierMap);
                dBlockItem->setDeclaration(resolvedDeclaration);
            }
            else if (auto functionDeclaration =
                         std::dynamic_pointer_cast<FunctionDeclaration>(
                             dBlockItem->getDeclaration())) {
                auto resolvedDeclaration = resolveFunctionDeclaration(
                    functionDeclaration, identifierMap);
                dBlockItem->setDeclaration(resolvedDeclaration);
            }
            else {
                throw std::runtime_error("Unsupported declaration type");
            }
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            auto resolvedStatement =
                resolveStatement(sBlockItem->getStatement(), identifierMap);
            sBlockItem->setStatement(resolvedStatement);
        }
        else {
            throw std::runtime_error("Unsupported block item type");
        }
    }

    // Return a new block with the resolved block items.
    return std::make_shared<Block>(std::move(blockItems));
}

std::shared_ptr<ForInit> IdentifierResolutionPass::resolveForInit(
    std::shared_ptr<ForInit> forInit,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Resolve the for-init based on the type of the for init.
    if (auto initExpr = std::dynamic_pointer_cast<InitExpr>(forInit)) {
        auto optExpr = initExpr->getExpression();
        if (optExpr.has_value()) {
            auto resolvedExpr =
                resolveExpression(optExpr.value(), identifierMap);
            return std::make_shared<InitExpr>(std::move(resolvedExpr));
        }
        else {
            return std::make_shared<InitExpr>();
        }
    }
    else if (auto initDecl = std::dynamic_pointer_cast<InitDecl>(forInit)) {
        auto resolvedDecl = resolveVariableDeclaration(
            initDecl->getVariableDeclaration(), identifierMap);
        return std::make_shared<InitDecl>(std::move(resolvedDecl));
    }
    else {
        throw std::runtime_error("Unsupported for init type");
    }
}

std::shared_ptr<FunctionDeclaration>
IdentifierResolutionPass::resolveFunctionDeclaration(
    std::shared_ptr<FunctionDeclaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(declaration->getIdentifier()) !=
        identifierMap.end()) {
        auto previousEntry = identifierMap[declaration->getIdentifier()];
        if (previousEntry.fromCurrentBlockOrNot() &&
            !previousEntry.hasLinkageOrNot()) {
            std::stringstream msg;
            msg << "Duplicate function declaration: "
                << declaration->getIdentifier();
            throw std::runtime_error(msg.str());
        }
    }
    identifierMap[declaration->getIdentifier()] =
        MapEntry(declaration->getIdentifier(), true, true);
    auto innerIdentifierMap = copyIdentifierMap(identifierMap);
    auto resolvedParameters = std::make_shared<std::vector<std::string>>();
    for (auto &parameter : *declaration->getParameters()) {
        if (parameter == "int") {
            continue;
        }
        resolvedParameters->emplace_back(
            resolveParameter(parameter, innerIdentifierMap));
    }
    auto resolvedOptBody = std::optional<std::shared_ptr<Block>>();
    if (declaration->getOptBody().has_value()) {
        resolvedOptBody = std::make_optional(resolveBlock(
            declaration->getOptBody().value(), innerIdentifierMap));
    }
    return std::make_shared<FunctionDeclaration>(declaration->getIdentifier(),
                                                 std::move(resolvedParameters),
                                                 std::move(resolvedOptBody));
}

std::string IdentifierResolutionPass::resolveParameter(
    std::string parameter,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(parameter) != identifierMap.end()) {
        auto previousEntry = identifierMap[parameter];
        if (previousEntry.fromCurrentBlockOrNot()) {
            std::stringstream msg;
            msg << "Duplicate parameter declaration: " << parameter;
            throw std::runtime_error(msg.str());
        }
    }
    auto uniqueVariableName = generateUniqueVariableName(parameter);
    identifierMap[parameter] = MapEntry(uniqueVariableName, true, true);
    return uniqueVariableName;
}
/*
 * End: Functions for the identifier-resolution pass.
 */

/*
 * Start: Functions for the loop-labeling pass.
 */
void LoopLabelingPass::labelLoops(std::shared_ptr<Program> program) {
    // TODO(zzmic): This is a temporary solution.
    for (auto &functionDeclaration : *program->getFunctionDeclarations()) {
        if (functionDeclaration->getOptBody().has_value()) {
            auto resolvedBody =
                labelBlock(functionDeclaration->getOptBody().value(), "");
            functionDeclaration->setOptBody(std::make_optional(resolvedBody));
        }
    }
}

std::string LoopLabelingPass::generateLoopLabel() {
    // Return a new label with the current counter value.
    return "loop" + std::to_string(this->loopLabelingCounter++);
}

std::shared_ptr<Statement>
LoopLabelingPass::annotateStatement(std::shared_ptr<Statement> statement,
                                    std::string label) {
    if (auto breakStatement =
            std::dynamic_pointer_cast<BreakStatement>(statement)) {
        breakStatement->setLabel(label);
        return breakStatement;
    }
    else if (auto continueStatement =
                 std::dynamic_pointer_cast<ContinueStatement>(statement)) {
        continueStatement->setLabel(label);
        return continueStatement;
    }
    else if (auto whileStatement =
                 std::dynamic_pointer_cast<WhileStatement>(statement)) {
        whileStatement->setLabel(label);
        return whileStatement;
    }
    else if (auto doWhileStatement =
                 std::dynamic_pointer_cast<DoWhileStatement>(statement)) {
        doWhileStatement->setLabel(label);
        return doWhileStatement;
    }
    else if (auto forStatement =
                 std::dynamic_pointer_cast<ForStatement>(statement)) {
        forStatement->setLabel(label);
        return forStatement;
    }
    else {
        return statement;
    }
}

std::shared_ptr<Statement>
LoopLabelingPass::labelStatement(std::shared_ptr<Statement> statement,
                                 std::string label) {
    if (auto breakStatement =
            std::dynamic_pointer_cast<BreakStatement>(statement)) {
        if (label == "") {
            throw std::runtime_error("Break statement outside of loop");
        }
        return annotateStatement(std::move(breakStatement), label);
    }
    else if (auto continueStatement =
                 std::dynamic_pointer_cast<ContinueStatement>(statement)) {
        if (label == "") {
            throw std::runtime_error("Continue statement outside of loop");
        }
        return annotateStatement(std::move(continueStatement), label);
    }
    else if (auto whileStatement =
                 std::dynamic_pointer_cast<WhileStatement>(statement)) {
        auto newLabel = generateLoopLabel();
        auto labeledBody = labelStatement(whileStatement->getBody(), newLabel);
        auto labeledWhileStatement = std::make_shared<WhileStatement>(
            whileStatement->getCondition(), std::move(labeledBody));
        return annotateStatement(std::move(labeledWhileStatement), newLabel);
    }
    else if (auto doWhileStatement =
                 std::dynamic_pointer_cast<DoWhileStatement>(statement)) {
        auto newLabel = generateLoopLabel();
        auto labeledBody =
            labelStatement(doWhileStatement->getBody(), newLabel);
        auto labeledDoWhileStatement = std::make_shared<DoWhileStatement>(
            doWhileStatement->getCondition(), std::move(labeledBody));
        return annotateStatement(std::move(labeledDoWhileStatement), newLabel);
    }
    else if (auto forStatement =
                 std::dynamic_pointer_cast<ForStatement>(statement)) {
        auto newLabel = generateLoopLabel();
        auto labeledBody = labelStatement(forStatement->getBody(), newLabel);
        auto labeledForStatement = std::make_shared<ForStatement>(
            forStatement->getForInit(), forStatement->getOptCondition(),
            forStatement->getOptPost(), std::move(labeledBody));
        return annotateStatement(std::move(labeledForStatement), newLabel);
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        auto labeledThenStatement =
            labelStatement(ifStatement->getThenStatement(), label);
        if (ifStatement->getElseOptStatement().has_value()) {
            auto labeledElseStatement = labelStatement(
                ifStatement->getElseOptStatement().value(), label);
            return std::make_shared<IfStatement>(
                ifStatement->getCondition(), std::move(labeledThenStatement),
                std::move(labeledElseStatement));
        }
        else {
            return std::make_shared<IfStatement>(
                ifStatement->getCondition(), std::move(labeledThenStatement));
        }
    }
    else if (auto compoundStatement =
                 std::dynamic_pointer_cast<CompoundStatement>(statement)) {
        auto block = compoundStatement->getBlock();
        auto labeledBlock = labelBlock(block, label);
        return std::make_shared<CompoundStatement>(std::move(labeledBlock));
    }
    else {
        return statement;
    }
}

std::shared_ptr<Block>
LoopLabelingPass::labelBlock(std::shared_ptr<Block> block, std::string label) {
    auto blockItems = block->getBlockItems();
    auto newBlockItems =
        std::make_shared<std::vector<std::shared_ptr<BlockItem>>>();
    for (auto &blockItem : *blockItems) {
        if (auto dBlockItem =
                std::dynamic_pointer_cast<DBlockItem>(blockItem)) {
            newBlockItems->emplace_back(dBlockItem);
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            auto resolvedStatement =
                labelStatement(sBlockItem->getStatement(), label);
            auto labeledSBlockItem =
                std::make_shared<SBlockItem>(std::move(resolvedStatement));
            newBlockItems->emplace_back(std::move(labeledSBlockItem));
        }
        else {
            throw std::runtime_error("Unsupported block item type");
        }
    }
    return std::make_shared<Block>(std::move(newBlockItems));
}
/*
 * End: Functions for the loop-labeling pass.
 */
} // namespace AST
