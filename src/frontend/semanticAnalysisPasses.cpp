#include "semanticAnalysisPasses.h"
#include "frontendSymbolTable.h"
#include <memory>
#include <sstream>

namespace AST {
/*
 * Start: Functions for the identifier-resolution pass.
 */
std::pair<std::unique_ptr<Program>, int>
IdentifierResolutionPass::resolveProgram(std::unique_ptr<Program> program) {
    // Initialize an empty identifier map (that will be passed to the helpers).
    // Instead of maintaining a "global" identifier map, we pass the identifier
    // map to the helper functions to, together with `copyIdentifierMap`, ensure
    // that each identifier map is corresponding to a specific block.
    auto identifierMap = std::unordered_map<std::string, MapEntry>();

    // At the top level, resolve the list of declarations in the
    // program.
    std::vector<std::unique_ptr<Declaration>> resolvedDeclarations;
    auto &declarations = program->getDeclarations();
    for (auto &declaration : declarations) {
        if (dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            auto funcDeclPtr = std::unique_ptr<FunctionDeclaration>(
                static_cast<FunctionDeclaration *>(declaration.release()));
            auto resolvedFunctionDeclaration = resolveFunctionDeclaration(
                std::move(funcDeclPtr), identifierMap);
            resolvedDeclarations.emplace_back(
                std::move(resolvedFunctionDeclaration));
        }
        else if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
            auto varDeclPtr = std::unique_ptr<VariableDeclaration>(
                static_cast<VariableDeclaration *>(declaration.release()));
            auto resolvedVariableDeclaration =
                resolveFileScopeVariableDeclaration(std::move(varDeclPtr),
                                                    identifierMap);
            resolvedDeclarations.emplace_back(
                std::move(resolvedVariableDeclaration));
        }
        else {
            throw std::logic_error(
                "Unsupported declaration type for identifier resolution");
        }
    }
    program->setDeclarations(std::move(resolvedDeclarations));

    return std::make_pair(std::move(program), this->variableResolutionCounter);
}

std::unordered_map<std::string, MapEntry>
IdentifierResolutionPass::copyIdentifierMap(
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Make a copy of the identifier map and set the `fromCurrentScope` flag to
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

std::unique_ptr<Declaration>
IdentifierResolutionPass::resolveFileScopeVariableDeclaration(
    std::unique_ptr<Declaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
        auto varDeclPtr = std::unique_ptr<VariableDeclaration>(
            static_cast<VariableDeclaration *>(declaration.release()));
        identifierMap[varDeclPtr->getIdentifier()] =
            MapEntry(varDeclPtr->getIdentifier(), true, true);
        return varDeclPtr;
    }
    else {
        throw std::logic_error(
            "Unsupported declaration type for file-scope variable resolution");
    }
}

std::unique_ptr<VariableDeclaration>
IdentifierResolutionPass::resolveLocalVariableDeclaration(
    std::unique_ptr<VariableDeclaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(declaration->getIdentifier()) !=
        identifierMap.end()) {
        auto previousEntry = identifierMap[declaration->getIdentifier()];
        if (previousEntry.fromCurrentScopeOrNot()) {
            if (!(previousEntry.hasLinkageOrNot() &&
                  declaration->getOptStorageClass().has_value() &&
                  dynamic_cast<ExternStorageClass *>(
                      declaration->getOptStorageClass().value().get()))) {
                std::stringstream msg;
                msg << "Conflicting local variable declaration: "
                    << declaration->getIdentifier();
                throw std::logic_error(msg.str());
            }
        }
    }
    if (declaration->getOptStorageClass().has_value() &&
        dynamic_cast<ExternStorageClass *>(
            declaration->getOptStorageClass().value().get())) {
        identifierMap[declaration->getIdentifier()] =
            MapEntry(declaration->getIdentifier(), true, true);
        return declaration;
    }
    else {
        auto declarationIdentifier = declaration->getIdentifier();
        auto uniqueVariableName =
            generateUniqueVariableName(declarationIdentifier);
        identifierMap[declarationIdentifier] =
            MapEntry(uniqueVariableName, true, false);
        auto &optInitializer = declaration->getOptInitializer();
        if (optInitializer.has_value()) {
            auto resolvedInitializer = resolveExpression(
                std::move(optInitializer.value()), identifierMap);
            optInitializer = std::make_optional(std::move(resolvedInitializer));
        }
        return std::make_unique<VariableDeclaration>(
            identifierMap[declarationIdentifier].getNewName(),
            std::move(optInitializer), std::move(declaration->getVarType()),
            std::move(declaration->getOptStorageClass()));
    }
}

std::unique_ptr<Statement> IdentifierResolutionPass::resolveStatement(
    std::unique_ptr<Statement> statement,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (dynamic_cast<ReturnStatement *>(statement.get())) {
        auto returnStatementPtr = std::unique_ptr<ReturnStatement>(
            static_cast<ReturnStatement *>(statement.release()));
        // If the statement is a return statement, resolve the expression in the
        // return statement.
        auto resolvedExpression = resolveExpression(
            std::move(returnStatementPtr->getExpression()), identifierMap);
        // Return a new return statement with the resolved expression.
        return std::make_unique<ReturnStatement>(std::move(resolvedExpression));
    }
    else if (dynamic_cast<ExpressionStatement *>(statement.get())) {
        auto expressionStatementPtr = std::unique_ptr<ExpressionStatement>(
            static_cast<ExpressionStatement *>(statement.release()));
        // If the statement is an expression statement, resolve the expression
        // in the expression statement.
        auto resolvedExpression = resolveExpression(
            std::move(expressionStatementPtr->getExpression()), identifierMap);
        // Return a new expression statement with the resolved expression.
        return std::make_unique<ExpressionStatement>(
            std::move(resolvedExpression));
    }
    else if (dynamic_cast<CompoundStatement *>(statement.get())) {
        auto compoundStatementPtr = std::unique_ptr<CompoundStatement>(
            static_cast<CompoundStatement *>(statement.release()));
        // Copy the identifier map (with modifications) and resolve the block in
        // the compound statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        auto resolvedBlock =
            resolveBlock(compoundStatementPtr->getBlock(), copiedIdentifierMap);
        return std::make_unique<CompoundStatement>(std::move(resolvedBlock));
    }
    else if (dynamic_cast<BreakStatement *>(statement.get())) {
        auto breakStatementPtr = std::unique_ptr<BreakStatement>(
            static_cast<BreakStatement *>(statement.release()));
        // If the statement is a break statement, return the break statement.
        return breakStatementPtr;
    }
    else if (dynamic_cast<ContinueStatement *>(statement.get())) {
        auto continueStatementPtr = std::unique_ptr<ContinueStatement>(
            static_cast<ContinueStatement *>(statement.release()));
        // If the statement is a continue statement, return the continue
        // statement.
        return continueStatementPtr;
    }
    else if (dynamic_cast<WhileStatement *>(statement.get())) {
        auto whileStatementPtr = std::unique_ptr<WhileStatement>(
            static_cast<WhileStatement *>(statement.release()));
        // If the statement is a while-statement, resolve the condition
        // expression and the body statement in the while-statement.
        auto resolvedCondition = resolveExpression(
            std::move(whileStatementPtr->getCondition()), identifierMap);
        auto resolvedBody = resolveStatement(
            std::move(whileStatementPtr->getBody()), identifierMap);
        return std::make_unique<WhileStatement>(std::move(resolvedCondition),
                                                std::move(resolvedBody));
    }
    else if (dynamic_cast<DoWhileStatement *>(statement.get())) {
        auto doWhileStatementPtr = std::unique_ptr<DoWhileStatement>(
            static_cast<DoWhileStatement *>(statement.release()));
        // If the statement is a do-while-statement, resolve the condition
        // expression and the body statement in the do-while-statement.
        auto resolvedCondition = resolveExpression(
            std::move(doWhileStatementPtr->getCondition()), identifierMap);
        auto resolvedBody = resolveStatement(
            std::move(doWhileStatementPtr->getBody()), identifierMap);
        return std::make_unique<DoWhileStatement>(std::move(resolvedCondition),
                                                  std::move(resolvedBody));
    }
    else if (dynamic_cast<ForStatement *>(statement.get())) {
        auto forStatementPtr = std::unique_ptr<ForStatement>(
            static_cast<ForStatement *>(statement.release()));
        // Copy the identifier map (with modifications) and resolve the
        // for-init, (optional) condition, (optional) post, and body in the
        // for-statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        auto resolvedForInit = resolveForInit(
            std::move(forStatementPtr->getForInit()), copiedIdentifierMap);
        auto resolvedOptCondition =
            std::optional<std::unique_ptr<Expression>>();
        if (forStatementPtr->getOptCondition().has_value()) {
            auto resolvedCondition = resolveExpression(
                std::move(forStatementPtr->getOptCondition().value()),
                copiedIdentifierMap);
            resolvedOptCondition =
                std::make_optional(std::move(resolvedCondition));
        }
        auto resolvedOptPost = std::optional<std::unique_ptr<Expression>>();
        if (forStatementPtr->getOptPost().has_value()) {
            auto resolvedPost = resolveExpression(
                std::move(forStatementPtr->getOptPost().value()),
                copiedIdentifierMap);
            resolvedOptPost = std::make_optional(std::move(resolvedPost));
        }
        auto resolvedBody = resolveStatement(
            std::move(forStatementPtr->getBody()), copiedIdentifierMap);
        return std::make_unique<ForStatement>(
            std::move(resolvedForInit), std::move(resolvedOptCondition),
            std::move(resolvedOptPost), std::move(resolvedBody));
    }
    else if (dynamic_cast<IfStatement *>(statement.get())) {
        auto ifStatementPtr = std::unique_ptr<IfStatement>(
            static_cast<IfStatement *>(statement.release()));
        // If the statement is an if-statement, resolve the condition
        // expression, then-statement, and (optional) else-statement in the
        // if-statement.
        auto resolvedCondition = resolveExpression(
            std::move(ifStatementPtr->getCondition()), identifierMap);
        auto resolvedThenStatement = resolveStatement(
            std::move(ifStatementPtr->getThenStatement()), identifierMap);
        if (ifStatementPtr->getElseOptStatement().has_value()) {
            auto resolvedElseStatement = resolveStatement(
                std::move(ifStatementPtr->getElseOptStatement().value()),
                identifierMap);
            return std::make_unique<IfStatement>(
                std::move(resolvedCondition), std::move(resolvedThenStatement),
                std::move(resolvedElseStatement));
        }
        else {
            return std::make_unique<IfStatement>(
                std::move(resolvedCondition), std::move(resolvedThenStatement));
        }
    }
    else if (dynamic_cast<NullStatement *>(statement.get())) {
        auto nullStatementPtr = std::unique_ptr<NullStatement>(
            static_cast<NullStatement *>(statement.release()));
        // If the statement is a null statement, return the null statement.
        return nullStatementPtr;
    }
    else {
        throw std::logic_error(
            "Unsupported statement type for identifier resolution");
    }
}

std::unique_ptr<Expression> IdentifierResolutionPass::resolveExpression(
    std::unique_ptr<Expression> expression,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (dynamic_cast<AssignmentExpression *>(expression.get())) {
        auto assignmentExpressionPtr = std::unique_ptr<AssignmentExpression>(
            static_cast<AssignmentExpression *>(expression.release()));
        if (!(dynamic_cast<VariableExpression *>(
                assignmentExpressionPtr->getLeft().get()))) {
            throw std::logic_error("Invalid lvalue in assignment expression");
        }
        auto resolvedLeft = resolveExpression(
            std::move(assignmentExpressionPtr->getLeft()), identifierMap);
        auto resolvedRight = resolveExpression(
            std::move(assignmentExpressionPtr->getRight()), identifierMap);
        return std::make_unique<AssignmentExpression>(std::move(resolvedLeft),
                                                      std::move(resolvedRight));
    }
    else if (dynamic_cast<VariableExpression *>(expression.get())) {
        // If the expression is a variable expression, check if the variable is
        // already in the identifier map. If it is not, throw an error.
        // Otherwise, return a new variable expression with the resolved
        // identifier from the identifier map.
        auto variableExpressionPtr = std::unique_ptr<VariableExpression>(
            static_cast<VariableExpression *>(expression.release()));
        if (identifierMap.find(variableExpressionPtr->getIdentifier()) ==
            identifierMap.end()) {
            std::stringstream msg;
            msg << "Undeclared variable: "
                << variableExpressionPtr->getIdentifier();
            throw std::logic_error(msg.str());
        }
        return std::make_unique<VariableExpression>(
            identifierMap[variableExpressionPtr->getIdentifier()].getNewName());
    }
    else if (dynamic_cast<ConstantExpression *>(expression.get())) {
        // If the expression is a constant expression, return the constant
        // expression.
        auto constantExpressionPtr = std::unique_ptr<ConstantExpression>(
            static_cast<ConstantExpression *>(expression.release()));
        return constantExpressionPtr;
    }
    else if (dynamic_cast<UnaryExpression *>(expression.get())) {
        // If the expression is a unary expression, resolve the expression in
        // the unary expression.
        auto unaryExpressionPtr = std::unique_ptr<UnaryExpression>(
            static_cast<UnaryExpression *>(expression.release()));
        auto resolvedExpression = resolveExpression(
            std::move(unaryExpressionPtr->getExpression()), identifierMap);
        if (dynamic_cast<Factor *>(resolvedExpression.get())) {
            auto factorPtr = std::unique_ptr<Factor>(
                static_cast<Factor *>(resolvedExpression.release()));
            return std::make_unique<UnaryExpression>(
                std::move(unaryExpressionPtr->getOperator()),
                std::move(factorPtr));
        }
        else {
            throw std::logic_error(
                "Unsupported expression type for unary expression identifier "
                "resolution");
        }
    }
    else if (dynamic_cast<BinaryExpression *>(expression.get())) {
        // If the expression is a binary expression, resolve the left and right
        // expressions in the binary expression.
        auto binaryExpressionPtr = std::unique_ptr<BinaryExpression>(
            static_cast<BinaryExpression *>(expression.release()));
        auto resolvedLeft = resolveExpression(
            std::move(binaryExpressionPtr->getLeft()), identifierMap);
        auto resolvedRight = resolveExpression(
            std::move(binaryExpressionPtr->getRight()), identifierMap);
        if (dynamic_cast<BinaryOperator *>(
                binaryExpressionPtr->getOperator().get())) {
            auto binaryOperatorPtr =
                std::unique_ptr<BinaryOperator>(static_cast<BinaryOperator *>(
                    binaryExpressionPtr->getOperator().release()));
            return std::make_unique<BinaryExpression>(
                std::move(resolvedLeft), std::move(binaryOperatorPtr),
                std::move(resolvedRight));
        }
        else {
            throw std::logic_error(
                "Unsupported operator type for binary expression identifier "
                "resolution");
        }
    }
    else if (dynamic_cast<ConditionalExpression *>(expression.get())) {
        // If the expression is a conditional expression, resolve the condition
        // expression, then-expression, and else-expression in the conditional
        // expression.
        auto conditionalExpressionPtr = std::unique_ptr<ConditionalExpression>(
            static_cast<ConditionalExpression *>(expression.release()));
        auto resolvedCondition = resolveExpression(
            std::move(conditionalExpressionPtr->getCondition()), identifierMap);
        auto resolvedThenExpression = resolveExpression(
            std::move(conditionalExpressionPtr->getThenExpression()),
            identifierMap);
        auto resolvedElseExpression = resolveExpression(
            std::move(conditionalExpressionPtr->getElseExpression()),
            identifierMap);
        // Return a new conditional expression with the resolved condition
        // expression, then-expression, and else-expression.
        return std::make_unique<ConditionalExpression>(
            std::move(resolvedCondition), std::move(resolvedThenExpression),
            std::move(resolvedElseExpression));
    }
    else if (dynamic_cast<FunctionCallExpression *>(expression.get())) {
        // If the expression is a function call expression, check if the
        // function is already in the identifier map. If it is not, throw an
        // error. Otherwise, return a new function call expression with the
        // resolved identifier from the identifier map and the resolved
        // arguments.
        auto functionCallExpressionPtr =
            std::unique_ptr<FunctionCallExpression>(
                static_cast<FunctionCallExpression *>(expression.release()));
        if (identifierMap.find(functionCallExpressionPtr->getIdentifier()) ==
            identifierMap.end()) {
            std::stringstream msg;
            msg << "Undeclared function: "
                << functionCallExpressionPtr->getIdentifier();
            throw std::logic_error(msg.str());
        }
        auto resolvedFunctionName =
            identifierMap[functionCallExpressionPtr->getIdentifier()]
                .getNewName();
        auto resolvedArguments = std::vector<std::unique_ptr<Expression>>();
        auto &oldArguments = functionCallExpressionPtr->getArguments();
        for (auto &argument : oldArguments) {
            auto resolvedArgument =
                resolveExpression(std::move(argument), identifierMap);
            resolvedArguments.emplace_back(std::move(resolvedArgument));
        }
        return std::make_unique<FunctionCallExpression>(
            std::move(resolvedFunctionName), std::move(resolvedArguments));
    }
    else if (dynamic_cast<CastExpression *>(expression.get())) {
        auto castExpressionPtr = std::unique_ptr<CastExpression>(
            static_cast<CastExpression *>(expression.release()));
        auto resolvedExpression = resolveExpression(
            std::move(castExpressionPtr->getExpression()), identifierMap);
        return std::make_unique<CastExpression>(
            std::move(castExpressionPtr->getTargetType()),
            std::move(resolvedExpression));
    }
    else {
        throw std::logic_error(
            "Unsupported expression type for identifier resolution");
    }
}

Block *IdentifierResolutionPass::resolveBlock(
    Block *block, std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Get the block items from the block and resolve the variables in each
    // block item.
    auto &blockItems = block->getBlockItems();
    for (auto &blockItem : blockItems) {
        if (dynamic_cast<DBlockItem *>(blockItem.get())) {
            auto dBlockItemPtr = std::unique_ptr<DBlockItem>(
                static_cast<DBlockItem *>(blockItem.release()));
            auto declaration = std::move(dBlockItemPtr->getDeclaration());
            if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
                auto variableDeclarationPtr =
                    std::unique_ptr<VariableDeclaration>(
                        static_cast<VariableDeclaration *>(
                            declaration.release()));
                auto resolvedDeclaration = resolveLocalVariableDeclaration(
                    std::move(variableDeclarationPtr), identifierMap);
                dBlockItemPtr->setDeclaration(std::move(resolvedDeclaration));
            }
            else if (dynamic_cast<FunctionDeclaration *>(declaration.get())) {
                auto functionDeclarationPtr =
                    std::unique_ptr<FunctionDeclaration>(
                        static_cast<FunctionDeclaration *>(
                            declaration.release()));
                auto resolvedDeclaration = resolveFunctionDeclaration(
                    std::move(functionDeclarationPtr), identifierMap);
                dBlockItemPtr->setDeclaration(std::move(resolvedDeclaration));
            }
            else {
                throw std::logic_error(
                    "Unsupported declaration type for identifier resolution");
            }
        }
        else if (dynamic_cast<SBlockItem *>(blockItem.get())) {
            auto sBlockItemPtr = std::unique_ptr<SBlockItem>(
                static_cast<SBlockItem *>(blockItem.release()));
            auto resolvedStatement = resolveStatement(
                std::move(sBlockItemPtr->getStatement()), identifierMap);
            sBlockItemPtr->setStatement(std::move(resolvedStatement));
        }
        else {
            throw std::logic_error(
                "Unsupported block item typen for identifier resolution");
        }
    }

    // Return a new block with the resolved block items.
    return new Block(std::move(blockItems));
}

std::unique_ptr<ForInit> IdentifierResolutionPass::resolveForInit(
    std::unique_ptr<ForInit> forInit,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Resolve the for-init based on the type of the for init.
    if (dynamic_cast<InitExpr *>(forInit.get())) {
        auto initExprPtr = std::unique_ptr<InitExpr>(
            static_cast<InitExpr *>(forInit.release()));
        auto optExpr = std::move(initExprPtr->getExpression());
        if (optExpr.has_value()) {
            auto resolvedExpr =
                resolveExpression(std::move(optExpr.value()), identifierMap);
            return std::make_unique<InitExpr>(std::move(resolvedExpr));
        }
        else {
            return std::make_unique<InitExpr>();
        }
    }
    else if (dynamic_cast<InitDecl *>(forInit.get())) {
        auto initDeclPtr = std::unique_ptr<InitDecl>(
            static_cast<InitDecl *>(forInit.release()));
        auto resolvedDecl = resolveLocalVariableDeclaration(
            std::move(initDeclPtr->getVariableDeclaration()), identifierMap);
        return std::make_unique<InitDecl>(std::move(resolvedDecl));
    }
    else {
        throw std::logic_error(
            "Unsupported for-init type for identifier resolution");
    }
}

std::unique_ptr<FunctionDeclaration>
IdentifierResolutionPass::resolveFunctionDeclaration(
    std::unique_ptr<FunctionDeclaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(declaration->getIdentifier()) !=
        identifierMap.end()) {
        auto previousEntry = identifierMap[declaration->getIdentifier()];
        if (previousEntry.fromCurrentScopeOrNot() &&
            !previousEntry.hasLinkageOrNot()) {
            std::stringstream msg;
            msg << "Duplicate function declaration: "
                << declaration->getIdentifier();
            throw std::logic_error(msg.str());
        }
    }
    identifierMap[declaration->getIdentifier()] =
        MapEntry(declaration->getIdentifier(), true, true);
    auto innerIdentifierMap = copyIdentifierMap(identifierMap);
    auto resolvedParameters = std::vector<std::string>();
    for (auto &parameter : declaration->getParameterIdentifiers()) {
        // Skip the built-in types `int` and `long` since they are not actual
        // parameters.
        if (parameter == "int" || parameter == "long") {
            continue;
        }
        resolvedParameters.emplace_back(
            resolveParameter(parameter, innerIdentifierMap));
    }
    auto resolvedOptBody = std::optional<Block *>();
    if (declaration->getOptBody().has_value()) {
        resolvedOptBody = std::make_optional(resolveBlock(
            declaration->getOptBody().value(), innerIdentifierMap));
    }
    return std::make_unique<FunctionDeclaration>(
        declaration->getIdentifier(), std::move(resolvedParameters),
        std::move(resolvedOptBody), std::move(declaration->getFunType()),
        std::move(declaration->getOptStorageClass()));
}

std::string IdentifierResolutionPass::resolveParameter(
    std::string parameter,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(parameter) != identifierMap.end()) {
        auto previousEntry = identifierMap[parameter];
        if (previousEntry.fromCurrentScopeOrNot()) {
            std::stringstream msg;
            msg << "Duplicate parameter declaration: " << parameter;
            throw std::logic_error(msg.str());
        }
    }
    auto uniqueVariableName = generateUniqueVariableName(parameter);
    // A parameter should be considered in the current scope and should not have
    // linkage.
    identifierMap[parameter] = MapEntry(uniqueVariableName, true, false);
    return uniqueVariableName;
}
/*
 * End: Functions for the identifier-resolution pass.
 */

/*
 * Start: Functions for the type-checking pass.
 */
std::unique_ptr<Program>
TypeCheckingPass::typeCheckProgram(std::unique_ptr<Program> program) {
    // Clear the symbol table for this compilation.
    frontendSymbolTable.clear();

    // Type-check the program.
    for (auto &declaration : program->getDeclarations()) {
        if (dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            auto functionDeclarationPtr = std::unique_ptr<FunctionDeclaration>(
                static_cast<FunctionDeclaration *>(declaration.release()));
            auto resolvedFunctionDeclaration =
                typeCheckFunctionDeclaration(std::move(functionDeclarationPtr));
            program->getDeclarations().emplace_back(
                std::move(resolvedFunctionDeclaration));
        }
        else if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
            auto variableDeclarationPtr = std::unique_ptr<VariableDeclaration>(
                static_cast<VariableDeclaration *>(declaration.release()));
            auto resolvedVariableDeclaration =
                typeCheckFileScopeVariableDeclaration(
                    std::move(variableDeclarationPtr));
            program->getDeclarations().emplace_back(
                std::move(resolvedVariableDeclaration));
        }
        else {
            throw std::logic_error(
                "Unsupported declaration type for type checking at top level");
        }
    }

    // Return the type-checked program.
    return program;
}

std::unique_ptr<StaticInit> TypeCheckingPass::convertStaticConstantToStaticInit(
    std::unique_ptr<Type> varType,
    std::unique_ptr<ConstantExpression> constantExpression) {
    // Extract the numeric value from the AST constant.
    auto variantValue = constantExpression->getConstantInIntOrLongVariant();

    long numericValue = 0;
    // Convert everything to 64-bit for uniform handling.
    // `std::holds_alternative` checks the type of the variant.
    if (std::holds_alternative<int>(variantValue)) {
        numericValue = std::get<int>(variantValue);
    }
    else {
        numericValue = std::get<long>(variantValue);
    }

    // If the declared type is int, we wrap/truncate to 32-bit.
    if (*varType == IntType()) {
        // Wrap `numericValue` via a cast.
        // This does a 32-bit wrap-around if `numericValue` doesn't fit.
        int wrappedNumericValue =
            static_cast<int>(static_cast<unsigned long>(numericValue));
        return std::make_unique<IntInit>(wrappedNumericValue);
    }
    // If the declared type is long, store the full 64-bit value.
    else if (*varType == LongType()) {
        return std::make_unique<LongInit>(numericValue);
    }
    // Otherwise, throw an error.
    else {
        throw std::logic_error("Unsupported type in static initializer");
    }
}

std::unique_ptr<Type>
TypeCheckingPass::getCommonType(std::unique_ptr<Type> type1,
                                std::unique_ptr<Type> type2) {
    // If `type1` is `nullptr`, throw an error.
    if (type1 == nullptr) {
        throw std::logic_error("Null type1 in getCommonType");
    }
    // TODO(zzmic): Check if this is correct.
    // If `type2` is `nullptr`, return `type1`.
    else if (type2 == nullptr) {
        return type1;
    }
    // If both types are the same, return `type1` (or `type2`).
    // For now, there are only two primitive types: `int` and `long`.
    else if (*type1 == *type2) {
        return type1;
    }
    // Otherwise, return the larger type.
    else {
        return std::make_unique<LongType>();
    }
}

std::unique_ptr<Expression>
TypeCheckingPass::convertTo(std::unique_ptr<Expression> expression,
                            std::unique_ptr<Type> targetType) {
    if (expression == nullptr) {
        throw std::logic_error("Null expression in convertTo");
    }
    if (targetType == nullptr) {
        throw std::logic_error("Null target type in convertTo");
    }
    auto expressionType = std::move(expression->getExpType());
    if (expressionType == nullptr) {
        throw std::logic_error("Null expression type in convertTo");
    }
    // If the expression type is the same as the target type, return the
    // expression.
    if (*expressionType == *targetType) {
        return expression;
    }
    // Otherwise, wrap the expression in a cast expression and return the cast
    // expression.
    auto castExpression = std::make_unique<CastExpression>(
        std::move(targetType), std::move(expression));
    return castExpression;
}

std::unique_ptr<FunctionDeclaration>
TypeCheckingPass::typeCheckFunctionDeclaration(
    std::unique_ptr<FunctionDeclaration> declaration) {
    auto funType = std::move(declaration->getFunType());
    auto funTypePtr = std::unique_ptr<FunctionType>(
        static_cast<FunctionType *>(funType.release()));
    if (funTypePtr == nullptr) {
        throw std::logic_error("Function type is not a FunctionType");
    }

    auto parameterTypes = std::move(funTypePtr->getParameterTypes());
    auto returnType = std::move(funTypePtr->getReturnType());
    auto hasBody = declaration->getOptBody().has_value();
    auto alreadyDefined = false;
    auto global = true;

    if (declaration->getOptStorageClass().has_value() &&
        dynamic_cast<StaticStorageClass *>(
            declaration->getOptStorageClass().value().get())) {
        global = false;
    }

    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto oldDeclaration =
            std::move(frontendSymbolTable.at(declaration->getIdentifier()));
        auto oldType = std::move(oldDeclaration.first);
        if (*oldType != *funType) {
            throw std::logic_error("Incompatible function declarations");
        }
        auto oldFunctionAttribute = std::unique_ptr<FunctionAttribute>(
            static_cast<FunctionAttribute *>(oldDeclaration.second.release()));
        alreadyDefined = oldFunctionAttribute->isDefined();
        if (alreadyDefined && hasBody) {
            throw std::logic_error("Function redefinition");
        }
        if (oldFunctionAttribute->isGlobal() &&
            declaration->getOptStorageClass().has_value() &&
            dynamic_cast<StaticStorageClass *>(
                declaration->getOptStorageClass().value().get())) {
            throw std::logic_error(
                "Static function declaration follows non-static");
        }
        global = oldFunctionAttribute->isGlobal();
    }

    // Create a function attribute and store it in the symbol table.
    auto attribute =
        std::make_unique<FunctionAttribute>(alreadyDefined || hasBody, global);
    frontendSymbolTable[declaration->getIdentifier()] = {std::move(funType),
                                                         std::move(attribute)};

    // If the function has a body, we need to set the parameter types in the
    // symbol table.
    if (hasBody) {
        const auto &funcParameterTypes = funTypePtr->getParameterTypes();
        auto parameterIdentifiers =
            std::move(declaration->getParameterIdentifiers());
        // Set parameter types in the symbol table based on the function's
        // parameter types.
        for (size_t i = 0; i < parameterIdentifiers.size(); ++i) {
            // If the parameter type is available, use it.
            if (i < funcParameterTypes.size()) {
                // Create a "copy" that resonates the parameter type.
                std::unique_ptr<Type> paramTypeCopy;
                if (dynamic_cast<IntType *>(funcParameterTypes[i].get())) {
                    paramTypeCopy = std::make_unique<IntType>();
                }
                else if (dynamic_cast<LongType *>(
                             funcParameterTypes[i].get())) {
                    paramTypeCopy = std::make_unique<LongType>();
                }
                else {
                    // Fallback to `IntType`.
                    paramTypeCopy = std::make_unique<IntType>();
                }
                frontendSymbolTable[parameterIdentifiers[i]] = {
                    std::move(paramTypeCopy),
                    std::make_unique<LocalAttribute>()};
            }
            // Otherwise, fallback to `IntType`.
            // TODO(zzmic): Check if this should be retained or be replaced by
            // an exception being thrown.
            else {
                frontendSymbolTable[parameterIdentifiers[i]] = {
                    std::make_unique<IntType>(),
                    std::make_unique<LocalAttribute>()};
            }
        }
        // Provide the enclosing function's name for the later type-checking of
        // the return statement.
        auto resolvedBody =
            typeCheckBlock(std::move(declaration->getOptBody().value()),
                           declaration->getIdentifier());
        return std::make_unique<FunctionDeclaration>(
            declaration->getIdentifier(), std::move(parameterIdentifiers),
            std::move(resolvedBody), std::move(funType),
            std::move(declaration->getOptStorageClass()));
    }
    else {
        return std::make_unique<FunctionDeclaration>(
            declaration->getIdentifier(),
            std::move(declaration->getParameterIdentifiers()),
            std::move(funType), std::move(declaration->getOptStorageClass()));
    }
}

std::unique_ptr<VariableDeclaration>
TypeCheckingPass::typeCheckFileScopeVariableDeclaration(
    std::unique_ptr<VariableDeclaration> declaration) {
    auto varType = std::move(declaration->getVarType());
    if (*varType != IntType() && *varType != LongType()) {
        throw std::logic_error(
            "Unsupported variable type for file-scope variables");
    }

    std::unique_ptr<InitialValue> initialValue;
    if (declaration->getOptInitializer().has_value() &&
        dynamic_cast<ConstantExpression *>(
            declaration->getOptInitializer().value().get())) {
        auto constantExpressionPtr = std::unique_ptr<ConstantExpression>(
            static_cast<ConstantExpression *>(
                declaration->getOptInitializer().value().release()));
        auto variantValue =
            constantExpressionPtr->getConstantInIntOrLongVariant();
        if (std::holds_alternative<long>(variantValue)) {
            initialValue =
                std::make_unique<Initial>(std::get<long>(variantValue));
        }
        else if (std::holds_alternative<int>(variantValue)) {
            initialValue =
                std::make_unique<Initial>(std::get<int>(variantValue));
        }
        else {
            throw std::logic_error("Unsupported type in static initializer");
        }
    }
    else if (!declaration->getOptInitializer().has_value()) {
        if (declaration->getOptStorageClass().has_value() &&
            dynamic_cast<ExternStorageClass *>(
                declaration->getOptStorageClass().value().get())) {
            initialValue = std::make_unique<NoInitializer>();
        }
        else {
            initialValue = std::make_unique<Tentative>();
        }
    }
    else {
        throw std::logic_error("Non-constant initializer!");
    }

    // Determine the linkage of the variable.
    auto global = (!declaration->getOptStorageClass().has_value()) ||
                  (declaration->getOptStorageClass().has_value() &&
                   !(dynamic_cast<StaticStorageClass *>(
                       declaration->getOptStorageClass().value().get())));

    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto oldDeclaration =
            std::move(frontendSymbolTable.at(declaration->getIdentifier()));
        auto oldType = std::move(oldDeclaration.first);
        if (*oldType != *varType) {
            throw std::logic_error("Function redeclared as variable");
        }
        auto oldStaticAttribute = std::unique_ptr<StaticAttribute>(
            static_cast<StaticAttribute *>(oldDeclaration.second.release()));
        if (declaration->getOptStorageClass().has_value() &&
            dynamic_cast<ExternStorageClass *>(
                declaration->getOptStorageClass().value().get())) {
            global = oldStaticAttribute->isGlobal();
        }
        else if (oldStaticAttribute->isGlobal() != global) {
            throw std::logic_error("Conflicting variable linkage");
        }
        if (dynamic_cast<Initial *>(
                oldStaticAttribute->getInitialValue().get())) {
            if (dynamic_cast<Initial *>(initialValue.get())) {
                throw std::logic_error(
                    "Conflicting file-scope variable definitions");
            }
            else {
                initialValue = std::move(oldStaticAttribute->getInitialValue());
            }
        }
        else if (!dynamic_cast<Initial *>(initialValue.get()) &&
                 dynamic_cast<Tentative *>(
                     oldStaticAttribute->getInitialValue().get())) {
            initialValue = std::make_unique<Tentative>();
        }
    }

    // Create a static attribute and store it in the symbol table.
    auto attribute =
        std::make_unique<StaticAttribute>(std::move(initialValue), global);
    frontendSymbolTable[declaration->getIdentifier()] = {std::move(varType),
                                                         std::move(attribute)};

    return std::make_unique<VariableDeclaration>(
        declaration->getIdentifier(), std::move(varType),
        std::move(declaration->getOptStorageClass()));
}

std::unique_ptr<VariableDeclaration>
TypeCheckingPass::typeCheckLocalVariableDeclaration(
    std::unique_ptr<VariableDeclaration> declaration) {
    auto varType = std::move(declaration->getVarType());
    if (*varType != IntType() && *varType != LongType()) {
        throw std::logic_error("Unsupported variable type for local variables");
    }

    if (declaration->getOptStorageClass().has_value() &&
        dynamic_cast<ExternStorageClass *>(
            declaration->getOptStorageClass().value().get())) {
        if (declaration->getOptInitializer().has_value()) {
            throw std::logic_error(
                "Initializer on local extern variable declaration");
        }
        if (frontendSymbolTable.find(declaration->getIdentifier()) !=
            frontendSymbolTable.end()) {
            auto oldDeclaration =
                std::move(frontendSymbolTable.at(declaration->getIdentifier()));
            auto oldType = std::move(oldDeclaration.first);
            if (*oldType != *varType) {
                throw std::logic_error("Function redeclared as variable");
            }
        }
        else {
            auto staticAttribute = std::make_unique<StaticAttribute>(
                std::make_unique<NoInitializer>(), true);
            frontendSymbolTable[declaration->getIdentifier()] = {
                std::move(varType), std::move(staticAttribute)};
        }
        return std::make_unique<VariableDeclaration>(
            declaration->getIdentifier(), std::move(varType),
            std::move(declaration->getOptStorageClass()));
    }
    else if (declaration->getOptStorageClass().has_value() &&
             dynamic_cast<StaticStorageClass *>(
                 declaration->getOptStorageClass().value().get())) {
        std::unique_ptr<InitialValue> initialValue;
        if (declaration->getOptInitializer().has_value() &&
            dynamic_cast<ConstantExpression *>(
                declaration->getOptInitializer().value().get())) {
            auto constantExpressionPtr = std::unique_ptr<ConstantExpression>(
                static_cast<ConstantExpression *>(
                    declaration->getOptInitializer().value().release()));
            auto variantValue =
                constantExpressionPtr->getConstantInIntOrLongVariant();
            if (std::holds_alternative<long>(variantValue)) {
                initialValue =
                    std::make_unique<Initial>(std::get<long>(variantValue));
            }
            else if (std::holds_alternative<int>(variantValue)) {
                initialValue =
                    std::make_unique<Initial>(std::get<int>(variantValue));
            }
            else {
                throw std::logic_error(
                    "Unsupported type in static initializer");
            }
        }
        else if (!declaration->getOptInitializer().has_value()) {
            initialValue = std::make_unique<Initial>(0);
        }
        else {
            throw std::logic_error(
                "Non-constant initializer on local static variable");
        }
        auto staticAttribute =
            std::make_unique<StaticAttribute>(std::move(initialValue), false);
        frontendSymbolTable[declaration->getIdentifier()] = {
            std::move(varType), std::move(staticAttribute)};
        return std::make_unique<VariableDeclaration>(
            declaration->getIdentifier(), std::move(varType),
            std::move(declaration->getOptStorageClass()));
    }
    else {
        auto localAttribute = std::make_unique<LocalAttribute>();
        frontendSymbolTable[declaration->getIdentifier()] = {
            std::move(varType), std::move(localAttribute)};
        if (declaration->getOptInitializer().has_value()) {
            auto resolvedInitializer = typeCheckExpression(
                std::move(declaration->getOptInitializer().value()));
            return std::make_unique<VariableDeclaration>(
                declaration->getIdentifier(), std::move(resolvedInitializer),
                std::move(varType),
                std::move(declaration->getOptStorageClass()));
        }
        else {
            return std::make_unique<VariableDeclaration>(
                declaration->getIdentifier(), std::move(varType),
                std::move(declaration->getOptStorageClass()));
        }
    }
}

Block *
TypeCheckingPass::typeCheckBlock(Block *block,
                                 std::string enclosingFunctionIdentifier) {
    auto &blockItems = block->getBlockItems();
    for (auto &blockItem : blockItems) {
        if (dynamic_cast<DBlockItem *>(blockItem.get())) {
            auto dBlockItemPtr = std::unique_ptr<DBlockItem>(
                static_cast<DBlockItem *>(blockItem.release()));
            auto declaration = std::move(dBlockItemPtr->getDeclaration());
            if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
                auto variableDeclarationPtr =
                    std::unique_ptr<VariableDeclaration>(
                        static_cast<VariableDeclaration *>(
                            declaration.release()));
                auto resolvedDeclaration = typeCheckLocalVariableDeclaration(
                    std::move(variableDeclarationPtr));
                dBlockItemPtr->setDeclaration(std::move(resolvedDeclaration));
            }
            else if (dynamic_cast<FunctionDeclaration *>(declaration.get())) {
                auto functionDeclarationPtr =
                    std::unique_ptr<FunctionDeclaration>(
                        static_cast<FunctionDeclaration *>(
                            declaration.release()));
                if (functionDeclarationPtr->getOptBody().has_value()) {
                    throw std::logic_error(
                        "Nested function definitions are not permitted");
                }
                if (functionDeclarationPtr->getOptStorageClass().has_value() &&
                    dynamic_cast<StaticStorageClass *>(
                        functionDeclarationPtr->getOptStorageClass()
                            .value()
                            .get())) {
                    throw std::logic_error(
                        "Static storage class on block-scope function "
                        "declaration");
                }
                auto resolvedDeclaration = typeCheckFunctionDeclaration(
                    std::move(functionDeclarationPtr));
                dBlockItemPtr->setDeclaration(std::move(resolvedDeclaration));
            }
            else {
                throw std::logic_error(
                    "Unsupported declaration type for type-checking");
            }
        }
        else if (dynamic_cast<SBlockItem *>(blockItem.get())) {
            auto sBlockItemPtr = std::unique_ptr<SBlockItem>(
                static_cast<SBlockItem *>(blockItem.release()));
            // Provide the enclosing function's name for the later type-checking
            // of the return statement.
            auto resolvedStatement =
                typeCheckStatement(std::move(sBlockItemPtr->getStatement()),
                                   enclosingFunctionIdentifier);
            sBlockItemPtr->setStatement(std::move(resolvedStatement));
        }
        else {
            throw std::logic_error(
                "Unsupported block item type for type-checking");
        }
    }

    return new Block(std::move(blockItems));
}

std::unique_ptr<Expression>
TypeCheckingPass::typeCheckExpression(std::unique_ptr<Expression> expression) {
    if (dynamic_cast<FunctionCallExpression *>(expression.get())) {
        auto functionCallExpressionPtr =
            std::unique_ptr<FunctionCallExpression>(
                static_cast<FunctionCallExpression *>(expression.release()));
        auto fType = std::move(
            frontendSymbolTable.at(functionCallExpressionPtr->getIdentifier())
                .first);

        if (*fType == IntType() || *fType == LongType()) {
            throw std::logic_error("Function name used as variable: " +
                                   functionCallExpressionPtr->getIdentifier());
        }
        else {
            auto functionType = std::unique_ptr<FunctionType>(
                static_cast<FunctionType *>(fType.release()));
            auto &parameterTypes = functionType->getParameterTypes();
            auto &arguments = functionCallExpressionPtr->getArguments();
            if (parameterTypes.size() != arguments.size()) {
                throw std::logic_error(
                    "Function called with a wrong number of arguments");
            }
            auto convertedArguments =
                std::vector<std::unique_ptr<Expression>>();
            // Iterate over the function's arguments and parameters together.
            // Type-check each argument and convert it to the corresponding
            // parameter type.
            for (size_t i = 0; i < arguments.size(); ++i) {
                auto argument = std::move(arguments[i]);
                auto parameterType = std::move(parameterTypes[i]);
                auto resolvedArgument =
                    typeCheckExpression(std::move(argument));
                auto convertedArgument = convertTo(std::move(resolvedArgument),
                                                   std::move(parameterType));
                convertedArguments.emplace_back(std::move(convertedArgument));
            }
            // Set the function call expression's arguments to the converted
            // arguments.
            functionCallExpressionPtr->setArguments(
                std::move(convertedArguments));
            // Set the function call expression's type to the function's return
            // type.
            functionCallExpressionPtr->setExpType(
                std::move(functionType->getReturnType()));
        }

        return functionCallExpressionPtr;
    }
    else if (dynamic_cast<ConstantExpression *>(expression.get())) {
        auto constantExpressionPtr = std::unique_ptr<ConstantExpression>(
            static_cast<ConstantExpression *>(expression.release()));
        auto constant = std::move(constantExpressionPtr->getConstant());

        if (dynamic_cast<ConstantInt *>(constant.get())) {
            constantExpressionPtr->setExpType(std::make_unique<IntType>());
        }
        else if (dynamic_cast<ConstantLong *>(constant.get())) {
            constantExpressionPtr->setExpType(std::make_unique<LongType>());
        }
        else {
            throw std::logic_error("Unsupported constant type");
        }

        return constantExpressionPtr;
    }
    else if (dynamic_cast<VariableExpression *>(expression.get())) {
        auto variableExpressionPtr = std::unique_ptr<VariableExpression>(
            static_cast<VariableExpression *>(expression.release()));
        auto variableType = std::move(
            frontendSymbolTable[variableExpressionPtr->getIdentifier()].first);

        // If the variable is not of type int or long, it is of type function.
        if (*variableType != IntType() && *variableType != LongType()) {
            std::stringstream msg;
            msg << "Function name used as variable: "
                << variableExpressionPtr->getIdentifier();
            throw std::logic_error(msg.str());
        }
        // Otherwise, set the expression type to the variable type.
        variableExpressionPtr->setExpType(std::move(variableType));

        return variableExpressionPtr;
    }
    else if (dynamic_cast<CastExpression *>(expression.get())) {
        auto castExpressionPtr = std::unique_ptr<CastExpression>(
            static_cast<CastExpression *>(expression.release()));
        auto resolvedExpression =
            typeCheckExpression(std::move(castExpressionPtr->getExpression()));
        castExpressionPtr->setExpType(
            std::move(castExpressionPtr->getTargetType()));

        return castExpressionPtr;
    }
    else if (dynamic_cast<AssignmentExpression *>(expression.get())) {
        auto assignmentExpressionPtr = std::unique_ptr<AssignmentExpression>(
            static_cast<AssignmentExpression *>(expression.release()));
        auto left = std::move(assignmentExpressionPtr->getLeft());
        auto right = std::move(assignmentExpressionPtr->getRight());
        auto resolvedLeft = typeCheckExpression(std::move(left));
        auto resolvedRight = typeCheckExpression(std::move(right));
        assignmentExpressionPtr->setLeft(std::move(resolvedLeft));
        assignmentExpressionPtr->setRight(std::move(resolvedRight));
        auto leftType = std::move(resolvedLeft->getExpType());
        assignmentExpressionPtr->setExpType(std::move(leftType));

        return assignmentExpressionPtr;
    }
    else if (dynamic_cast<UnaryExpression *>(expression.get())) {
        auto unaryExpressionPtr = std::unique_ptr<UnaryExpression>(
            static_cast<UnaryExpression *>(expression.release()));
        auto resolvedExpression =
            typeCheckExpression(std::move(unaryExpressionPtr->getExpression()));
        unaryExpressionPtr->setExpression(std::unique_ptr<Factor>(
            static_cast<Factor *>(resolvedExpression.release())));

        if (dynamic_cast<NotOperator *>(
                unaryExpressionPtr->getOperator().get())) {
            unaryExpressionPtr->setExpType(std::make_unique<IntType>());
        }
        else {
            unaryExpressionPtr->setExpType(
                std::move(unaryExpressionPtr->getExpression()->getExpType()));
        }

        return unaryExpressionPtr;
    }
    else if (dynamic_cast<BinaryExpression *>(expression.get())) {
        auto binaryExpressionPtr = std::unique_ptr<BinaryExpression>(
            static_cast<BinaryExpression *>(expression.release()));
        auto resolvedLeft =
            typeCheckExpression(std::move(binaryExpressionPtr->getLeft()));
        auto resolvedRight =
            typeCheckExpression(std::move(binaryExpressionPtr->getRight()));
        binaryExpressionPtr->setLeft(std::move(resolvedLeft));
        binaryExpressionPtr->setRight(std::move(resolvedRight));
        auto binaryOperator = std::move(binaryExpressionPtr->getOperator());
        if (dynamic_cast<AndOperator *>(binaryOperator.get()) ||
            dynamic_cast<OrOperator *>(binaryOperator.get())) {
            // Logical operators should always return type `int`.
            binaryExpressionPtr->setExpType(std::make_unique<IntType>());
            return binaryExpressionPtr;
        }
        auto leftType = std::move(resolvedLeft->getExpType());
        auto rightType = std::move(resolvedRight->getExpType());
        auto commonType =
            getCommonType(std::move(leftType), std::move(rightType));
        auto convertedLeft =
            convertTo(std::move(resolvedLeft), std::move(commonType));
        auto convertedRight =
            convertTo(std::move(resolvedRight), std::move(commonType));
        binaryExpressionPtr->setLeft(std::move(convertedLeft));
        binaryExpressionPtr->setRight(std::move(convertedRight));
        if (dynamic_cast<AddOperator *>(binaryOperator.get()) ||
            dynamic_cast<SubtractOperator *>(binaryOperator.get()) ||
            dynamic_cast<MultiplyOperator *>(binaryOperator.get()) ||
            dynamic_cast<DivideOperator *>(binaryOperator.get()) ||
            dynamic_cast<RemainderOperator *>(binaryOperator.get())) {
            binaryExpressionPtr->setExpType(std::move(commonType));
        }
        else {
            binaryExpressionPtr->setExpType(std::make_unique<IntType>());
        }

        return binaryExpressionPtr;
    }
    else if (dynamic_cast<ConditionalExpression *>(expression.get())) {
        auto conditionalExpressionPtr = std::unique_ptr<ConditionalExpression>(
            static_cast<ConditionalExpression *>(expression.release()));
        auto resolvedCondition = typeCheckExpression(
            std::move(conditionalExpressionPtr->getCondition()));
        auto resolvedThen = typeCheckExpression(
            std::move(conditionalExpressionPtr->getThenExpression()));
        auto resolvedElse = typeCheckExpression(
            std::move(conditionalExpressionPtr->getElseExpression()));
        auto conditionType =
            std::move(conditionalExpressionPtr->getCondition()->getExpType());
        auto thenType = std::move(
            conditionalExpressionPtr->getThenExpression()->getExpType());
        auto elseType = std::move(
            conditionalExpressionPtr->getElseExpression()->getExpType());
        // Get the common type of the then and else expressions/branches.
        auto commonType =
            getCommonType(std::move(thenType), std::move(elseType));
        // Convert the then and else expressions to the common type.
        auto convertedThen =
            convertTo(std::move(resolvedThen), std::move(commonType));
        auto convertedElse =
            convertTo(std::move(resolvedElse), std::move(commonType));
        conditionalExpressionPtr->setThenExpression(std::move(convertedThen));
        conditionalExpressionPtr->setElseExpression(std::move(convertedElse));
        // Set the conditional expression type to the common type.
        conditionalExpressionPtr->setExpType(std::move(commonType));

        return conditionalExpressionPtr;
    }
    else {
        throw std::logic_error("Unsupported expression type for type-checking");
    }
}

std::unique_ptr<Statement>
TypeCheckingPass::typeCheckStatement(std::unique_ptr<Statement> statement,
                                     std::string enclosingFunctionIdentifier) {
    if (dynamic_cast<ReturnStatement *>(statement.get())) {
        auto returnStatementPtr = std::unique_ptr<ReturnStatement>(
            static_cast<ReturnStatement *>(statement.release()));
        // Look up the enclosing function's return type and convert the return
        // value to that type.
        // Use the enclosing function's name to look up the enclosing function's
        // return type.
        auto functionType =
            std::move(frontendSymbolTable[enclosingFunctionIdentifier].first);
        if (!functionType) {
            throw std::logic_error("Function not found in symbol table: " +
                                   enclosingFunctionIdentifier);
        }
        if (*functionType == IntType() || *functionType == LongType()) {
            throw std::logic_error("Function name used as variable: " +
                                   enclosingFunctionIdentifier);
        }
        auto returnType = std::unique_ptr<FunctionType>(
            static_cast<FunctionType *>(functionType.release()));
        if (returnStatementPtr->getExpression()) {
            auto resolvedExpression = typeCheckExpression(
                std::move(returnStatementPtr->getExpression()));
            auto convertedReturn =
                convertTo(std::move(resolvedExpression),
                          std::move(returnType->getReturnType()));
            returnStatementPtr->setExpression(std::move(convertedReturn));
            return returnStatementPtr;
        }
        return returnStatementPtr;
    }
    else if (dynamic_cast<ExpressionStatement *>(statement.get())) {
        auto expressionStatementPtr = std::unique_ptr<ExpressionStatement>(
            static_cast<ExpressionStatement *>(statement.release()));
        auto resolvedExpression = typeCheckExpression(
            std::move(expressionStatementPtr->getExpression()));
        expressionStatementPtr->setExpression(std::move(resolvedExpression));
        return expressionStatementPtr;
    }
    else if (dynamic_cast<CompoundStatement *>(statement.get())) {
        auto compoundStatementPtr = std::unique_ptr<CompoundStatement>(
            static_cast<CompoundStatement *>(statement.release()));
        auto resolvedBlock =
            typeCheckBlock(std::move(compoundStatementPtr->getBlock()),
                           enclosingFunctionIdentifier);
        compoundStatementPtr->setBlock(std::move(resolvedBlock));
        return compoundStatementPtr;
    }
    else if (dynamic_cast<WhileStatement *>(statement.get())) {
        auto whileStatementPtr = std::unique_ptr<WhileStatement>(
            static_cast<WhileStatement *>(statement.release()));
        auto resolvedCondition =
            typeCheckExpression(std::move(whileStatementPtr->getCondition()));
        auto resolvedBody =
            typeCheckStatement(std::move(whileStatementPtr->getBody()),
                               enclosingFunctionIdentifier);
        whileStatementPtr->setCondition(std::move(resolvedCondition));
        whileStatementPtr->setBody(std::move(resolvedBody));
        return whileStatementPtr;
    }
    else if (dynamic_cast<DoWhileStatement *>(statement.get())) {
        auto doWhileStatementPtr = std::unique_ptr<DoWhileStatement>(
            static_cast<DoWhileStatement *>(statement.release()));
        auto resolvedCondition =
            typeCheckExpression(std::move(doWhileStatementPtr->getCondition()));
        auto resolvedBody =
            typeCheckStatement(std::move(doWhileStatementPtr->getBody()),
                               enclosingFunctionIdentifier);
        doWhileStatementPtr->setCondition(std::move(resolvedCondition));
        doWhileStatementPtr->setBody(std::move(resolvedBody));
        return doWhileStatementPtr;
    }
    else if (dynamic_cast<ForStatement *>(statement.get())) {
        auto forStatementPtr = std::unique_ptr<ForStatement>(
            static_cast<ForStatement *>(statement.release()));

        if (forStatementPtr->getForInit()) {
            auto resolvedForInit =
                typeCheckForInit(std::move(forStatementPtr->getForInit()));
            forStatementPtr->setForInit(std::move(resolvedForInit));
        }
        if (forStatementPtr->getOptCondition().has_value()) {
            auto resolvedCondition = typeCheckExpression(
                std::move(forStatementPtr->getOptCondition().value()));
            auto resolvedOptCondition =
                std::make_optional(std::move(resolvedCondition));
            forStatementPtr->setOptCondition(std::move(resolvedOptCondition));
        }
        if (forStatementPtr->getOptPost().has_value()) {
            auto resolvedPost = typeCheckExpression(
                std::move(forStatementPtr->getOptPost().value()));
            auto resolvedOptPost = std::make_optional(std::move(resolvedPost));
            forStatementPtr->setOptPost(std::move(resolvedOptPost));
        }

        auto resolvedBody = typeCheckStatement(
            std::move(forStatementPtr->getBody()), enclosingFunctionIdentifier);
        forStatementPtr->setBody(std::move(resolvedBody));

        return forStatementPtr;
    }
    else if (dynamic_cast<IfStatement *>(statement.get())) {
        auto ifStatementPtr = std::unique_ptr<IfStatement>(
            static_cast<IfStatement *>(statement.release()));
        auto resolvedCondition =
            typeCheckExpression(std::move(ifStatementPtr->getCondition()));
        auto resolvedThen =
            typeCheckStatement(std::move(ifStatementPtr->getThenStatement()),
                               enclosingFunctionIdentifier);
        if (ifStatementPtr->getElseOptStatement().has_value()) {
            auto resolvedElse = typeCheckStatement(
                std::move(ifStatementPtr->getElseOptStatement().value()),
                enclosingFunctionIdentifier);
            ifStatementPtr->setElseOptStatement(std::move(resolvedElse));
        }
        ifStatementPtr->setCondition(std::move(resolvedCondition));
        ifStatementPtr->setThenStatement(std::move(resolvedThen));
        return ifStatementPtr;
    }
    else if (dynamic_cast<NullStatement *>(statement.get())) {
        auto nullStatementPtr = std::unique_ptr<NullStatement>(
            static_cast<NullStatement *>(statement.release()));
        return nullStatementPtr;
    }
    else {
        throw std::logic_error("Unsupported statement type for type-checking");
    }
}

std::unique_ptr<ForInit>
TypeCheckingPass::typeCheckForInit(std::unique_ptr<ForInit> forInit) {
    if (dynamic_cast<InitExpr *>(forInit.get())) {
        auto initExprPtr = std::unique_ptr<InitExpr>(
            static_cast<InitExpr *>(forInit.release()));
        if (initExprPtr->getExpression()) {
            auto resolvedExpression = typeCheckExpression(
                std::move(initExprPtr->getExpression().value()));
            return std::make_unique<InitExpr>(std::move(resolvedExpression));
        }
        else {
            return std::make_unique<InitExpr>();
        }
    }
    else if (dynamic_cast<InitDecl *>(forInit.get())) {
        auto initDeclPtr = std::unique_ptr<InitDecl>(
            static_cast<InitDecl *>(forInit.release()));
        if (initDeclPtr->getVariableDeclaration()
                ->getOptStorageClass()
                .has_value()) {
            throw std::logic_error("Storage class in for-init declaration");
        }
        auto resolvedDecl = typeCheckLocalVariableDeclaration(
            std::move(initDeclPtr->getVariableDeclaration()));
        return std::make_unique<InitDecl>(std::move(resolvedDecl));
    }
    else {
        throw std::logic_error("Unsupported for-init type for type-checking");
    }
}
/*
 * End: Functions for the type-checking pass.
 */

/*
 * Start: Functions for the loop-labeling pass.
 */
std::unique_ptr<Program>
LoopLabelingPass::labelLoops(std::unique_ptr<Program> program) {
    for (auto &declaration : program->getDeclarations()) {
        if (dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            auto functionDeclarationPtr = std::unique_ptr<FunctionDeclaration>(
                static_cast<FunctionDeclaration *>(declaration.release()));
            if (functionDeclarationPtr->getOptBody().has_value()) {
                auto resolvedBody = labelBlock(
                    functionDeclarationPtr->getOptBody().value(), "");
                functionDeclarationPtr->setOptBody(
                    std::make_optional(std::move(resolvedBody)));
            }
        }
    }
    return program;
}

std::string LoopLabelingPass::generateLoopLabel() {
    // Return a new label with the current counter value.
    return "loop" + std::to_string(this->loopLabelingCounter++);
}

std::unique_ptr<Statement>
LoopLabelingPass::annotateStatement(std::unique_ptr<Statement> statement,
                                    std::string label) {
    if (dynamic_cast<BreakStatement *>(statement.get())) {
        auto breakStatementPtr = std::unique_ptr<BreakStatement>(
            static_cast<BreakStatement *>(statement.release()));
        breakStatementPtr->setLabel(label);
        return breakStatementPtr;
    }
    else if (dynamic_cast<ContinueStatement *>(statement.get())) {
        auto continueStatementPtr = std::unique_ptr<ContinueStatement>(
            static_cast<ContinueStatement *>(statement.release()));
        continueStatementPtr->setLabel(label);
        return continueStatementPtr;
    }
    else if (dynamic_cast<WhileStatement *>(statement.get())) {
        auto whileStatementPtr = std::unique_ptr<WhileStatement>(
            static_cast<WhileStatement *>(statement.release()));
        whileStatementPtr->setLabel(label);
        return whileStatementPtr;
    }
    else if (dynamic_cast<DoWhileStatement *>(statement.get())) {
        auto doWhileStatementPtr = std::unique_ptr<DoWhileStatement>(
            static_cast<DoWhileStatement *>(statement.release()));
        doWhileStatementPtr->setLabel(label);
        return doWhileStatementPtr;
    }
    else if (dynamic_cast<ForStatement *>(statement.get())) {
        auto forStatementPtr = std::unique_ptr<ForStatement>(
            static_cast<ForStatement *>(statement.release()));
        forStatementPtr->setLabel(label);
        return forStatementPtr;
    }
    else {
        return statement;
    }
}

std::unique_ptr<Statement>
LoopLabelingPass::labelStatement(std::unique_ptr<Statement> statement,
                                 std::string label) {
    if (dynamic_cast<BreakStatement *>(statement.get())) {
        auto breakStatementPtr = std::unique_ptr<BreakStatement>(
            static_cast<BreakStatement *>(statement.release()));
        if (label == "") {
            throw std::logic_error("Break statement outside of loop");
        }
        return annotateStatement(std::move(breakStatementPtr), label);
    }
    else if (dynamic_cast<ContinueStatement *>(statement.get())) {
        auto continueStatementPtr = std::unique_ptr<ContinueStatement>(
            static_cast<ContinueStatement *>(statement.release()));
        if (label == "") {
            throw std::logic_error("Continue statement outside of loop");
        }
        return annotateStatement(std::move(continueStatementPtr), label);
    }
    else if (dynamic_cast<WhileStatement *>(statement.get())) {
        auto whileStatementPtr = std::unique_ptr<WhileStatement>(
            static_cast<WhileStatement *>(statement.release()));
        auto newLabel = generateLoopLabel();
        auto labeledBody =
            labelStatement(std::move(whileStatementPtr->getBody()), newLabel);
        auto labeledWhileStatement = std::make_unique<WhileStatement>(
            std::move(whileStatementPtr->getCondition()),
            std::move(labeledBody));
        return annotateStatement(std::move(labeledWhileStatement), newLabel);
    }
    else if (dynamic_cast<DoWhileStatement *>(statement.get())) {
        auto doWhileStatementPtr = std::unique_ptr<DoWhileStatement>(
            static_cast<DoWhileStatement *>(statement.release()));
        auto newLabel = generateLoopLabel();
        auto labeledBody =
            labelStatement(std::move(doWhileStatementPtr->getBody()), newLabel);
        auto labeledDoWhileStatement = std::make_unique<DoWhileStatement>(
            std::move(doWhileStatementPtr->getCondition()),
            std::move(labeledBody));
        return annotateStatement(std::move(labeledDoWhileStatement), newLabel);
    }
    else if (dynamic_cast<ForStatement *>(statement.get())) {
        auto forStatementPtr = std::unique_ptr<ForStatement>(
            static_cast<ForStatement *>(statement.release()));
        auto newLabel = generateLoopLabel();
        auto labeledBody =
            labelStatement(std::move(forStatementPtr->getBody()), newLabel);
        auto labeledForStatement = std::make_unique<ForStatement>(
            std::move(forStatementPtr->getForInit()),
            std::move(forStatementPtr->getOptCondition()),
            std::move(forStatementPtr->getOptPost()), std::move(labeledBody));
        return annotateStatement(std::move(labeledForStatement), newLabel);
    }
    else if (dynamic_cast<IfStatement *>(statement.get())) {
        auto ifStatementPtr = std::unique_ptr<IfStatement>(
            static_cast<IfStatement *>(statement.release()));
        auto labeledThenStatement = labelStatement(
            std::move(ifStatementPtr->getThenStatement()), label);
        if (ifStatementPtr->getElseOptStatement().has_value()) {
            auto labeledElseStatement = labelStatement(
                std::move(ifStatementPtr->getElseOptStatement().value()),
                label);
            return std::make_unique<IfStatement>(
                std::move(ifStatementPtr->getCondition()),
                std::move(labeledThenStatement),
                std::move(labeledElseStatement));
        }
        else {
            return std::make_unique<IfStatement>(
                std::move(ifStatementPtr->getCondition()),
                std::move(labeledThenStatement));
        }
    }
    else if (dynamic_cast<CompoundStatement *>(statement.get())) {
        auto compoundStatementPtr = std::unique_ptr<CompoundStatement>(
            static_cast<CompoundStatement *>(statement.release()));
        auto labeledBlock = labelBlock(compoundStatementPtr->getBlock(), label);
        return std::make_unique<CompoundStatement>(std::move(labeledBlock));
    }
    else {
        return statement;
    }
}

Block *LoopLabelingPass::labelBlock(Block *block, std::string label) {
    auto &blockItems = block->getBlockItems();
    auto newBlockItems = std::vector<std::unique_ptr<BlockItem>>();
    for (auto &blockItem : blockItems) {
        if (dynamic_cast<DBlockItem *>(blockItem.get())) {
            auto dBlockItemPtr = std::unique_ptr<DBlockItem>(
                static_cast<DBlockItem *>(blockItem.release()));
            newBlockItems.emplace_back(std::move(dBlockItemPtr));
        }
        else if (dynamic_cast<SBlockItem *>(blockItem.get())) {
            auto sBlockItemPtr = std::unique_ptr<SBlockItem>(
                static_cast<SBlockItem *>(blockItem.release()));
            auto resolvedStatement =
                labelStatement(std::move(sBlockItemPtr->getStatement()), label);
            auto labeledSBlockItemPtr =
                std::make_unique<SBlockItem>(std::move(resolvedStatement));
            newBlockItems.emplace_back(std::move(labeledSBlockItemPtr));
        }
        else {
            throw std::logic_error(
                "Unsupported block item type for loop labeling");
        }
    }
    return new Block(std::move(newBlockItems));
}
/*
 * End: Functions for the loop-labeling pass.
 */
} // namespace AST
