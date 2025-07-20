#include "semanticAnalysisPasses.h"
#include "frontendSymbolTable.h"
#include <memory>
#include <sstream>

namespace AST {
/*
 * Start: Functions for the identifier-resolution pass.
 */
int IdentifierResolutionPass::resolveProgram(std::shared_ptr<Program> program) {
    // Initialize an empty identifier map (that will be passed to the helpers).
    // Instead of maintaining a "global" identifier map, we pass the identifier
    // map to the helper functions to, together with `copyIdentifierMap`, ensure
    // that each identifier map is corresponding to a specific block.
    auto identifierMap = std::unordered_map<std::string, MapEntry>();

    // At the top level, resolve the list of declarations in the
    // program.
    auto resolvedDeclarations =
        std::make_shared<std::vector<std::shared_ptr<Declaration>>>();
    for (auto &declaration : *program->getDeclarations()) {
        if (auto functionDeclaration =
                std::dynamic_pointer_cast<FunctionDeclaration>(declaration)) {
            auto resolvedFunctionDeclaration =
                resolveFunctionDeclaration(functionDeclaration, identifierMap);
            resolvedDeclarations->emplace_back(resolvedFunctionDeclaration);
        }
        else if (auto variableDeclaration =
                     std::dynamic_pointer_cast<VariableDeclaration>(
                         declaration)) {
            auto resolvedVariableDeclaration =
                resolveFileScopeVariableDeclaration(variableDeclaration,
                                                    identifierMap);
            resolvedDeclarations->emplace_back(resolvedVariableDeclaration);
        }
        else {
            throw std::logic_error(
                "Unsupported declaration type for identifier resolution");
        }
    }
    program->setDeclarations(resolvedDeclarations);

    return this->variableResolutionCounter;
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

std::shared_ptr<Declaration>
IdentifierResolutionPass::resolveFileScopeVariableDeclaration(
    std::shared_ptr<Declaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto variableDeclaration =
            std::dynamic_pointer_cast<VariableDeclaration>(declaration)) {
        identifierMap[variableDeclaration->getIdentifier()] =
            MapEntry(variableDeclaration->getIdentifier(), true, true);
        return declaration;
    }
    else {
        throw std::logic_error(
            "Unsupported declaration type for file-scope variable resolution");
    }
}

std::shared_ptr<VariableDeclaration>
IdentifierResolutionPass::resolveLocalVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(declaration->getIdentifier()) !=
        identifierMap.end()) {
        auto previousEntry = identifierMap[declaration->getIdentifier()];
        if (previousEntry.fromCurrentScopeOrNot()) {
            if (!(previousEntry.hasLinkageOrNot() &&
                  declaration->getOptStorageClass().has_value() &&
                  std::dynamic_pointer_cast<ExternStorageClass>(
                      declaration->getOptStorageClass().value()))) {
                std::stringstream msg;
                msg << "Conflicting local variable declaration: "
                    << declaration->getIdentifier();
                throw std::logic_error(msg.str());
            }
        }
    }
    if (declaration->getOptStorageClass().has_value() &&
        std::dynamic_pointer_cast<ExternStorageClass>(
            declaration->getOptStorageClass().value())) {
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
        auto optInitializer = declaration->getOptInitializer();
        if (optInitializer.has_value()) {
            auto tmpInitializer =
                resolveExpression(optInitializer.value(), identifierMap);
            optInitializer = std::make_optional(tmpInitializer);
        }
        return std::make_shared<VariableDeclaration>(
            identifierMap[declarationIdentifier].getNewName(),
            std::move(optInitializer), declaration->getVarType(),
            declaration->getOptStorageClass());
    }
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
        throw std::logic_error(
            "Unsupported statement type for identifier resolution");
    }
}

std::shared_ptr<Expression> IdentifierResolutionPass::resolveExpression(
    std::shared_ptr<Expression> expression,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto assignmentExpression =
            std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        if (!(std::dynamic_pointer_cast<VariableExpression>(
                assignmentExpression->getLeft()))) {
            throw std::logic_error("Invalid lvalue in assignment expression");
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
            throw std::logic_error(msg.str());
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
            throw std::logic_error(msg.str());
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
    else if (auto castExpression =
                 std::dynamic_pointer_cast<CastExpression>(expression)) {
        auto resolvedExpression =
            resolveExpression(castExpression->getExpression(), identifierMap);
        return std::make_shared<CastExpression>(castExpression->getTargetType(),
                                                std::move(resolvedExpression));
    }
    else {
        throw std::logic_error(
            "Unsupported expression type for identifier resolution");
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
                auto resolvedDeclaration = resolveLocalVariableDeclaration(
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
                throw std::logic_error(
                    "Unsupported declaration type for identifier resolution");
            }
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            auto resolvedStatement =
                resolveStatement(sBlockItem->getStatement(), identifierMap);
            sBlockItem->setStatement(resolvedStatement);
        }
        else {
            throw std::logic_error(
                "Unsupported block item typen for identifier resolution");
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
        auto resolvedDecl = resolveLocalVariableDeclaration(
            initDecl->getVariableDeclaration(), identifierMap);
        return std::make_shared<InitDecl>(std::move(resolvedDecl));
    }
    else {
        throw std::logic_error(
            "Unsupported for-init type for identifier resolution");
    }
}

std::shared_ptr<FunctionDeclaration>
IdentifierResolutionPass::resolveFunctionDeclaration(
    std::shared_ptr<FunctionDeclaration> declaration,
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
    auto resolvedParameters = std::make_shared<std::vector<std::string>>();
    for (auto &parameter : *declaration->getParameterIdentifiers()) {
        // Skip the built-in types `int` and `long` since they are not actual
        // parameters.
        if (parameter == "int" || parameter == "long") {
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
    return std::make_shared<FunctionDeclaration>(
        declaration->getIdentifier(), std::move(resolvedParameters),
        std::move(resolvedOptBody), declaration->getFunType(),
        declaration->getOptStorageClass());
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
void TypeCheckingPass::typeCheckProgram(std::shared_ptr<Program> program) {
    // Clear the symbol table for this compilation.
    frontendSymbolTable.clear();

    // Type-check the program.
    for (auto &declaration : *program->getDeclarations()) {
        if (auto functionDeclaration =
                std::dynamic_pointer_cast<FunctionDeclaration>(declaration)) {
            typeCheckFunctionDeclaration(functionDeclaration);
        }
        else if (auto variableDeclaration =
                     std::dynamic_pointer_cast<VariableDeclaration>(
                         declaration)) {
            typeCheckFileScopeVariableDeclaration(variableDeclaration);
        }
        else {
            throw std::logic_error(
                "Unsupported declaration type for type checking at top level");
        }
    }
}

std::shared_ptr<StaticInit> TypeCheckingPass::convertStaticConstantToStaticInit(
    std::shared_ptr<Type> varType,
    std::shared_ptr<ConstantExpression> constantExpression) {
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
        return std::make_shared<IntInit>(wrappedNumericValue);
    }
    // If the declared type is long, store the full 64-bit value.
    else if (*varType == LongType()) {
        return std::make_shared<LongInit>(numericValue);
    }
    else {
        throw std::logic_error("Unsupported type in static initializer");
    }
}

std::shared_ptr<Type>
TypeCheckingPass::getCommonType(std::shared_ptr<Type> type1,
                                std::shared_ptr<Type> type2) {
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
        return std::make_shared<LongType>();
    }
}

std::shared_ptr<Expression>
TypeCheckingPass::convertTo(std::shared_ptr<Expression> expression,
                            std::shared_ptr<Type> targetType) {
    if (expression == nullptr) {
        throw std::logic_error("Null expression in convertTo");
    }
    if (targetType == nullptr) {
        throw std::logic_error("Null target type in convertTo");
    }
    if (expression->getExpType() == targetType) {
        return expression;
    }
    // Otherwise, wrap the expression in a cast expression and annotate the
    // result with the correct type.
    auto castExpression =
        std::make_shared<CastExpression>(targetType, expression);
    return castExpression;
}

void TypeCheckingPass::typeCheckFunctionDeclaration(
    std::shared_ptr<FunctionDeclaration> declaration) {
    auto funType = declaration->getFunType();
    auto funTypePtr = std::dynamic_pointer_cast<FunctionType>(funType);
    if (funTypePtr == nullptr) {
        throw std::logic_error("Function type is not a FunctionType");
    }
    auto parameterTypes = funTypePtr->getParameterTypes();
    auto returnType = funTypePtr->getReturnType();
    auto hasBody = declaration->getOptBody().has_value();
    auto alreadyDefined = false;
    auto global = true;

    if (declaration->getOptStorageClass().has_value() &&
        std::dynamic_pointer_cast<StaticStorageClass>(
            declaration->getOptStorageClass().value())) {
        global = false;
    }
    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto oldDeclaration = frontendSymbolTable[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first;
        if (*oldType != *funType) {
            throw std::logic_error("Incompatible function declarations");
        }
        auto oldFunctionAttribute =
            std::dynamic_pointer_cast<FunctionAttribute>(oldDeclaration.second);
        alreadyDefined = oldFunctionAttribute->isDefined();
        if (alreadyDefined && hasBody) {
            throw std::logic_error("Function redefinition");
        }
        if (oldFunctionAttribute->isGlobal() &&
            declaration->getOptStorageClass().has_value() &&
            std::dynamic_pointer_cast<StaticStorageClass>(
                declaration->getOptStorageClass().value())) {
            throw std::logic_error(
                "Static function declaration follows non-static");
        }
        global = oldFunctionAttribute->isGlobal();
    }

    auto attribute =
        std::make_shared<FunctionAttribute>(alreadyDefined || hasBody, global);
    frontendSymbolTable[declaration->getIdentifier()] = {funType, attribute};

    if (hasBody) {
        for (auto &parameter : *declaration->getParameterIdentifiers()) {
            frontendSymbolTable[parameter] = {
                std::make_shared<IntType>(),
                std::make_shared<LocalAttribute>()};
        }
        // Provide the enclosing function's name for the later type-checking of
        // the return statement.
        typeCheckBlock(declaration->getOptBody().value(),
                       declaration->getIdentifier());
    }
}

void TypeCheckingPass::typeCheckFileScopeVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration) {
    auto varType = declaration->getVarType();
    if (*varType != IntType() && *varType != LongType()) {
        throw std::logic_error(
            "Unsupported variable type for file-scope variables");
    }

    auto initialValue = std::make_shared<InitialValue>();

    if (declaration->getOptInitializer().has_value() &&
        std::dynamic_pointer_cast<ConstantExpression>(
            declaration->getOptInitializer().value())) {
        auto constantExpression = std::dynamic_pointer_cast<ConstantExpression>(
            declaration->getOptInitializer().value());
        auto variantValue = constantExpression->getConstantInIntOrLongVariant();
        if (std::holds_alternative<long>(variantValue)) {
            initialValue =
                std::make_shared<Initial>(std::get<long>(variantValue));
        }
        else if (std::holds_alternative<int>(variantValue)) {
            initialValue =
                std::make_shared<Initial>(std::get<int>(variantValue));
        }
        else {
            throw std::logic_error("Unsupported type in static initializer");
        }
    }
    else if (!declaration->getOptInitializer().has_value()) {
        if (declaration->getOptStorageClass().has_value() &&
            std::dynamic_pointer_cast<ExternStorageClass>(
                declaration->getOptStorageClass().value())) {
            initialValue = std::make_shared<NoInitializer>();
        }
        else {
            initialValue = std::make_shared<Tentative>();
        }
    }
    else {
        throw std::logic_error("Non-constant initializer!");
    }

    // Determine the linkage of the variable.
    auto global = (!declaration->getOptStorageClass().has_value()) ||
                  (declaration->getOptStorageClass().has_value() &&
                   !(std::dynamic_pointer_cast<StaticStorageClass>(
                       declaration->getOptStorageClass().value())));

    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto oldDeclaration = frontendSymbolTable[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first;
        if (*oldType != *varType) {
            throw std::logic_error("Function redeclared as variable");
        }
        auto oldStaticAttribute =
            std::dynamic_pointer_cast<StaticAttribute>(oldDeclaration.second);
        if (declaration->getOptStorageClass().has_value() &&
            std::dynamic_pointer_cast<ExternStorageClass>(
                declaration->getOptStorageClass().value())) {
            global = oldStaticAttribute->isGlobal();
        }
        else if (oldStaticAttribute->isGlobal() != global) {
            throw std::logic_error("Conflicting variable linkage");
        }
        if (auto oldInitialValue = std::dynamic_pointer_cast<Initial>(
                oldStaticAttribute->getInitialValue())) {
            if (std::dynamic_pointer_cast<Initial>(initialValue)) {
                throw std::logic_error(
                    "Conflicting file-scope variable definitions");
            }
            else {
                initialValue = oldInitialValue;
            }
        }
        else if (!std::dynamic_pointer_cast<Initial>(initialValue) &&
                 std::dynamic_pointer_cast<Tentative>(
                     oldStaticAttribute->getInitialValue())) {
            initialValue = std::make_shared<Tentative>();
        }
    }

    auto attribute = std::make_shared<StaticAttribute>(initialValue, global);
    // Store the corresponding variable type and attribute in the symbol table.
    frontendSymbolTable[declaration->getIdentifier()] = {varType, attribute};
}

void TypeCheckingPass::typeCheckLocalVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration) {
    auto varType = declaration->getVarType();
    if (*varType != IntType() && *varType != LongType()) {
        throw std::logic_error("Unsupported variable type for local variables");
    }

    if (declaration->getOptStorageClass().has_value() &&
        std::dynamic_pointer_cast<ExternStorageClass>(
            declaration->getOptStorageClass().value())) {
        if (declaration->getOptInitializer().has_value()) {
            throw std::logic_error(
                "Initializer on local extern variable declaration");
        }
        if (frontendSymbolTable.find(declaration->getIdentifier()) !=
            frontendSymbolTable.end()) {
            auto oldDeclaration =
                frontendSymbolTable[declaration->getIdentifier()];
            auto oldType = oldDeclaration.first;
            if (*oldType != *varType) {
                throw std::logic_error("Function redeclared as variable");
            }
        }
        else {
            auto staticAttribute = std::make_shared<StaticAttribute>(
                std::make_shared<NoInitializer>(), true);
            frontendSymbolTable[declaration->getIdentifier()] =
                std::make_pair(varType, staticAttribute);
        }
    }
    else if (declaration->getOptStorageClass().has_value() &&
             std::dynamic_pointer_cast<StaticStorageClass>(
                 declaration->getOptStorageClass().value())) {
        auto initialValue = std::make_shared<InitialValue>();
        if (declaration->getOptInitializer().has_value() &&
            std::dynamic_pointer_cast<ConstantExpression>(
                declaration->getOptInitializer().value())) {
            auto constantExpression =
                std::dynamic_pointer_cast<ConstantExpression>(
                    declaration->getOptInitializer().value());
            auto variantValue =
                constantExpression->getConstantInIntOrLongVariant();
            if (std::holds_alternative<long>(variantValue)) {
                initialValue =
                    std::make_shared<Initial>(std::get<long>(variantValue));
            }
            else if (std::holds_alternative<int>(variantValue)) {
                initialValue =
                    std::make_shared<Initial>(std::get<int>(variantValue));
            }
            else {
                throw std::logic_error(
                    "Unsupported type in static initializer");
            }
        }
        else if (!declaration->getOptInitializer().has_value()) {
            initialValue = std::make_shared<Initial>(0);
        }
        else {
            throw std::logic_error(
                "Non-constant initializer on local static variable");
        }
        auto staticAttribute =
            std::make_shared<StaticAttribute>(initialValue, false);
        frontendSymbolTable[declaration->getIdentifier()] =
            std::make_pair(varType, staticAttribute);
    }
    else {
        auto localAttribute = std::make_shared<LocalAttribute>();
        frontendSymbolTable[declaration->getIdentifier()] =
            std::make_pair(varType, localAttribute);
        if (declaration->getOptInitializer().has_value()) {
            typeCheckExpression(declaration->getOptInitializer().value());
        }
    }
}

void TypeCheckingPass::typeCheckBlock(std::shared_ptr<Block> block,
                                      std::string enclosingFunctionIdentifier) {
    for (auto &blockItem : *block->getBlockItems()) {
        if (auto dBlockItem =
                std::dynamic_pointer_cast<DBlockItem>(blockItem)) {
            if (auto variableDeclaration =
                    std::dynamic_pointer_cast<VariableDeclaration>(
                        dBlockItem->getDeclaration())) {
                typeCheckLocalVariableDeclaration(variableDeclaration);
            }
            else if (auto functionDeclaration =
                         std::dynamic_pointer_cast<FunctionDeclaration>(
                             dBlockItem->getDeclaration())) {
                if (functionDeclaration->getOptBody().has_value()) {
                    throw std::logic_error(
                        "Nested function definitions are not permitted");
                }
                if (functionDeclaration->getOptStorageClass().has_value() &&
                    std::dynamic_pointer_cast<StaticStorageClass>(
                        functionDeclaration->getOptStorageClass().value())) {
                    throw std::logic_error(
                        "Static storage class on block-scope function "
                        "declaration");
                }
                typeCheckFunctionDeclaration(functionDeclaration);
            }
            else {
                throw std::logic_error(
                    "Unsupported declaration type for type-checking");
            }
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            // Provide the enclosing function's name for the later type-checking
            // of the return statement.
            typeCheckStatement(sBlockItem->getStatement(),
                               enclosingFunctionIdentifier);
        }
        else {
            throw std::logic_error(
                "Unsupported block item type for type-checking");
        }
    }
}

void TypeCheckingPass::typeCheckExpression(
    std::shared_ptr<Expression> expression) {
    if (auto functionCallExpression =
            std::dynamic_pointer_cast<FunctionCallExpression>(expression)) {
        auto fType =
            frontendSymbolTable[functionCallExpression->getIdentifier()].first;
        if (*fType == IntType() || *fType == LongType()) {
            throw std::logic_error("Function name used as variable: " +
                                   functionCallExpression->getIdentifier());
        }
        else {
            auto functionType = std::dynamic_pointer_cast<FunctionType>(fType);
            auto parameterTypes = functionType->getParameterTypes();
            auto arguments = functionCallExpression->getArguments();
            if (parameterTypes->size() != arguments->size()) {
                throw std::logic_error(
                    "Function called with a wrong number of arguments");
            }
            auto convertedArguments =
                std::make_shared<std::vector<std::shared_ptr<Expression>>>();
            // Iterate over the function's arguments and parameters together.
            // Type-check each argument and convert it to the corresponding
            // parameter type.
            for (size_t i = 0; i < arguments->size(); ++i) {
                auto argument = arguments->at(i);
                auto parameterType = parameterTypes->at(i);
                typeCheckExpression(argument);
                convertTo(argument, parameterType);
                convertedArguments->emplace_back(argument);
            }
            // Set the function call expression's arguments to the converted
            // arguments.
            functionCallExpression->setArguments(convertedArguments);
            // Set the function call expression's type to the function's return
            // type.
            functionCallExpression->setExpType(functionType->getReturnType());
        }
    }
    else if (auto constantExpression =
                 std::dynamic_pointer_cast<ConstantExpression>(expression)) {
        auto constant = constantExpression->getConstant();
        if (std::dynamic_pointer_cast<ConstantInt>(constant)) {
            constantExpression->setExpType(std::make_shared<IntType>());
        }
        else if (std::dynamic_pointer_cast<ConstantLong>(constant)) {
            constantExpression->setExpType(std::make_shared<LongType>());
        }
        else {
            throw std::logic_error("Unsupported constant type");
        }
    }
    else if (auto variableExpression =
                 std::dynamic_pointer_cast<VariableExpression>(expression)) {
        auto variableType =
            frontendSymbolTable[variableExpression->getIdentifier()].first;
        // If the variable is not of type int or long, it is of type function.
        if (*variableType != IntType() && *variableType != LongType()) {
            std::stringstream msg;
            msg << "Function name used as variable: "
                << variableExpression->getIdentifier();
            throw std::logic_error(msg.str());
        }
        // Otherwise, set the expression type to the variable type.
        variableExpression->setExpType(variableType);
    }
    else if (auto castExpression =
                 std::dynamic_pointer_cast<CastExpression>(expression)) {
        typeCheckExpression(castExpression->getExpression());
        castExpression->setExpType(castExpression->getTargetType());
    }
    else if (auto assignmentExpression =
                 std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        auto left = assignmentExpression->getLeft();
        auto right = assignmentExpression->getRight();
        typeCheckExpression(left);
        typeCheckExpression(right);
        auto leftType = left->getExpType();
        right->setExpType(leftType);
        assignmentExpression->setExpType(leftType);
    }
    else if (auto unaryExpression =
                 std::dynamic_pointer_cast<UnaryExpression>(expression)) {
        typeCheckExpression(unaryExpression->getExpression());
        if (std::dynamic_pointer_cast<NotOperator>(
                unaryExpression->getOperator())) {
            unaryExpression->setExpType(std::make_shared<IntType>());
        }
        else {
            unaryExpression->setExpType(
                unaryExpression->getExpression()->getExpType());
        }
    }
    else if (auto binaryExpression =
                 std::dynamic_pointer_cast<BinaryExpression>(expression)) {
        typeCheckExpression(binaryExpression->getLeft());
        typeCheckExpression(binaryExpression->getRight());
        auto binaryOperator = binaryExpression->getOperator();
        if (std::dynamic_pointer_cast<AndOperator>(binaryOperator) ||
            std::dynamic_pointer_cast<OrOperator>(binaryOperator)) {
            // Logical operators should always return type `int`.
            binaryExpression->setExpType(std::make_shared<IntType>());
            return;
        }
        auto leftType = binaryExpression->getLeft()->getExpType();
        auto rightType = binaryExpression->getRight()->getExpType();
        auto commonType = getCommonType(leftType, rightType);
        auto convertedLeft = convertTo(binaryExpression->getLeft(), commonType);
        auto convertedRight =
            convertTo(binaryExpression->getRight(), commonType);
        binaryExpression->setLeft(convertedLeft);
        binaryExpression->setRight(convertedRight);
        if (std::dynamic_pointer_cast<AddOperator>(binaryOperator) ||
            std::dynamic_pointer_cast<SubtractOperator>(binaryOperator) ||
            std::dynamic_pointer_cast<MultiplyOperator>(binaryOperator) ||
            std::dynamic_pointer_cast<DivideOperator>(binaryOperator) ||
            std::dynamic_pointer_cast<RemainderOperator>(binaryOperator)) {
            binaryExpression->setExpType(commonType);
        }
        else {
            binaryExpression->setExpType(std::make_shared<IntType>());
        }
    }
    else if (auto conditionalExpression =
                 std::dynamic_pointer_cast<ConditionalExpression>(expression)) {
        typeCheckExpression(conditionalExpression->getCondition());
        typeCheckExpression(conditionalExpression->getThenExpression());
        typeCheckExpression(conditionalExpression->getElseExpression());
        auto conditionType =
            conditionalExpression->getCondition()->getExpType();
        auto thenType =
            conditionalExpression->getThenExpression()->getExpType();
        auto elseType =
            conditionalExpression->getElseExpression()->getExpType();
        // Get the common type of the then and else expressions/branches.
        auto commonType = getCommonType(thenType, elseType);
        // Convert the then and else expressions to the common type.
        auto convertedThen =
            convertTo(conditionalExpression->getThenExpression(), commonType);
        auto convertedElse =
            convertTo(conditionalExpression->getElseExpression(), commonType);
        conditionalExpression->setThenExpression(convertedThen);
        conditionalExpression->setElseExpression(convertedElse);
        // Set the conditional expression type to the common type.
        conditionalExpression->setExpType(commonType);
    }
}

void TypeCheckingPass::typeCheckStatement(
    std::shared_ptr<Statement> statement,
    std::string enclosingFunctionIdentifier) {
    if (auto returnStatement =
            std::dynamic_pointer_cast<ReturnStatement>(statement)) {
        // Look up the enclosing function's return type and convert the return
        // value to that type.
        // Use the enclosing function's name to look up the enclosing function's
        // return type.
        auto functionType =
            frontendSymbolTable[enclosingFunctionIdentifier].first;
        if (!functionType) {
            throw std::logic_error("Function not found in symbol table: " +
                                   enclosingFunctionIdentifier);
        }
        if (*functionType == IntType() || *functionType == LongType()) {
            throw std::logic_error("Function name used as variable: " +
                                   enclosingFunctionIdentifier);
        }
        auto returnType = std::dynamic_pointer_cast<FunctionType>(functionType);
        if (returnStatement->getExpression()) {
            typeCheckExpression(returnStatement->getExpression());
            auto convertedReturn = convertTo(returnStatement->getExpression(),
                                             returnType->getReturnType());
            returnStatement->setExpression(convertedReturn);
        }
    }
    else if (auto expressionStatement =
                 std::dynamic_pointer_cast<ExpressionStatement>(statement)) {
        typeCheckExpression(expressionStatement->getExpression());
    }
    else if (auto compoundStatement =
                 std::dynamic_pointer_cast<CompoundStatement>(statement)) {
        typeCheckBlock(compoundStatement->getBlock(),
                       enclosingFunctionIdentifier);
    }
    else if (auto whileStatement =
                 std::dynamic_pointer_cast<WhileStatement>(statement)) {
        typeCheckExpression(whileStatement->getCondition());
        typeCheckStatement(whileStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto doWhileStatement =
                 std::dynamic_pointer_cast<DoWhileStatement>(statement)) {
        typeCheckExpression(doWhileStatement->getCondition());
        typeCheckStatement(doWhileStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto forStatement =
                 std::dynamic_pointer_cast<ForStatement>(statement)) {
        if (forStatement->getForInit()) {
            typeCheckForInit(forStatement->getForInit());
        }
        if (forStatement->getOptCondition().has_value()) {
            typeCheckExpression(forStatement->getOptCondition().value());
        }
        if (forStatement->getOptPost().has_value()) {
            typeCheckExpression(forStatement->getOptPost().value());
        }
        typeCheckStatement(forStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        typeCheckExpression(ifStatement->getCondition());
        typeCheckStatement(ifStatement->getThenStatement(),
                           enclosingFunctionIdentifier);
        if (ifStatement->getElseOptStatement().has_value()) {
            typeCheckStatement(ifStatement->getElseOptStatement().value(),
                               enclosingFunctionIdentifier);
        }
    }
}

void TypeCheckingPass::typeCheckForInit(std::shared_ptr<ForInit> forInit) {
    if (auto initExpr = std::dynamic_pointer_cast<InitExpr>(forInit)) {
        if (initExpr->getExpression()) {
            typeCheckExpression(initExpr->getExpression().value());
        }
    }
    else if (auto initDecl = std::dynamic_pointer_cast<InitDecl>(forInit)) {
        if (initDecl->getVariableDeclaration()
                ->getOptStorageClass()
                .has_value()) {
            throw std::logic_error("Storage class in for-init declaration");
        }
        typeCheckLocalVariableDeclaration(initDecl->getVariableDeclaration());
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
void LoopLabelingPass::labelLoops(std::shared_ptr<Program> program) {
    for (auto &declaration : *program->getDeclarations()) {
        if (auto functionDeclaration =
                std::dynamic_pointer_cast<FunctionDeclaration>(declaration)) {
            if (functionDeclaration->getOptBody().has_value()) {
                auto resolvedBody =
                    labelBlock(functionDeclaration->getOptBody().value(), "");
                functionDeclaration->setOptBody(
                    std::make_optional(resolvedBody));
            }
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
            throw std::logic_error("Break statement outside of loop");
        }
        return annotateStatement(std::move(breakStatement), label);
    }
    else if (auto continueStatement =
                 std::dynamic_pointer_cast<ContinueStatement>(statement)) {
        if (label == "") {
            throw std::logic_error("Continue statement outside of loop");
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
            throw std::logic_error(
                "Unsupported block item type for loop labeling");
        }
    }
    return std::make_shared<Block>(std::move(newBlockItems));
}
/*
 * End: Functions for the loop-labeling pass.
 */
} // namespace AST
