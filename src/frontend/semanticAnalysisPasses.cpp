#include "semanticAnalysisPasses.h"
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
            throw std::runtime_error(
                "Unsupported declaration type for identifier resolution");
        }
    }
    program->setDeclarations(resolvedDeclarations);

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
        throw std::runtime_error(
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
        if (previousEntry.fromCurrentBlockOrNot()) {
            if (!(previousEntry.hasLinkageOrNot() &&
                  declaration->getOptStorageClass().has_value() &&
                  std::dynamic_pointer_cast<ExternStorageClass>(
                      declaration->getOptStorageClass().value()))) {
                std::stringstream msg;
                msg << "Conflicting local variable declaration: "
                    << declaration->getIdentifier();
                throw std::runtime_error(msg.str());
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
            std::move(optInitializer), declaration->getOptStorageClass());
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
        throw std::runtime_error(
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
        throw std::runtime_error(
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
                throw std::runtime_error(
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
            throw std::runtime_error(
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
        throw std::runtime_error(
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
    return std::make_shared<FunctionDeclaration>(
        declaration->getIdentifier(), std::move(resolvedParameters),
        std::move(resolvedOptBody), declaration->getOptStorageClass());
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
 * Start: Functions for the type-checking pass.
 */
std::unordered_map<std::string, std::pair<std::shared_ptr<Type>,
                                          std::shared_ptr<IdentifierAttribute>>>
TypeCheckingPass::typeCheckProgram(std::shared_ptr<Program> program) {
    this->symbols =
        std::unordered_map<std::string,
                           std::pair<std::shared_ptr<Type>,
                                     std::shared_ptr<IdentifierAttribute>>>();
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
            throw std::runtime_error(
                "Unsupported declaration type for type checking at top level");
        }
    }
    // The symbol table "will need to be accessible in later compiler passes."
    return this->symbols;
}

void TypeCheckingPass::typeCheckFunctionDeclaration(
    std::shared_ptr<FunctionDeclaration> declaration) {
    auto funType =
        std::make_shared<FunctionType>(declaration->getParameters()->size());
    auto hasBody = declaration->getOptBody().has_value();
    auto alreadyDefined = false;
    auto global = true;
    if (declaration->getOptStorageClass().has_value() &&
        std::dynamic_pointer_cast<StaticStorageClass>(
            declaration->getOptStorageClass().value())) {
        global = false;
    }
    if (symbols.find(declaration->getIdentifier()) != symbols.end()) {
        auto oldDeclaration = symbols[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first;
        if (*oldType != *funType) {
            throw std::runtime_error("Incompatible function declarations");
        }
        auto oldFunctionAttribute =
            std::dynamic_pointer_cast<FunctionAttribute>(oldDeclaration.second);
        alreadyDefined = oldFunctionAttribute->isDefined();
        if (alreadyDefined && hasBody) {
            throw std::runtime_error("Function redefinition");
        }
        if (oldFunctionAttribute->isGlobal() &&
            declaration->getOptStorageClass().has_value() &&
            std::dynamic_pointer_cast<StaticStorageClass>(
                declaration->getOptStorageClass().value())) {
            throw std::runtime_error(
                "Static function declaration follows non-static");
        }
        global = oldFunctionAttribute->isGlobal();
    }
    auto attribute =
        std::make_shared<FunctionAttribute>(alreadyDefined || hasBody, global);
    symbols[declaration->getIdentifier()] = {funType, attribute};
    if (hasBody) {
        for (auto &parameter : *declaration->getParameters()) {
            symbols[parameter] = {std::make_shared<IntType>(),
                                  std::make_shared<LocalAttribute>()};
        }
        typeCheckBlock(declaration->getOptBody().value());
    }
}

void TypeCheckingPass::typeCheckFileScopeVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration) {
    auto initialValue = std::make_shared<InitialValue>();
    if (declaration->getOptInitializer().has_value() &&
        std::dynamic_pointer_cast<ConstantExpression>(
            declaration->getOptInitializer().value())) {
        initialValue = std::make_shared<ConstantInitial>(
            std::dynamic_pointer_cast<ConstantExpression>(
                declaration->getOptInitializer().value())
                ->getValue());
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
        throw std::runtime_error("Non-constant initializer!");
    }
    auto global = (!declaration->getOptStorageClass().has_value()) ||
                  (declaration->getOptStorageClass().has_value() &&
                   !(std::dynamic_pointer_cast<StaticStorageClass>(
                       declaration->getOptStorageClass().value())));
    if (symbols.find(declaration->getIdentifier()) != symbols.end()) {
        auto oldDeclaration = symbols[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first;
        if (*oldType != IntType()) {
            throw std::runtime_error("Function redclared as variable");
        }
        auto oldStaticAttribute =
            std::dynamic_pointer_cast<StaticAttribute>(oldDeclaration.second);
        if (declaration->getOptStorageClass().has_value() &&
            std::dynamic_pointer_cast<ExternStorageClass>(
                declaration->getOptStorageClass().value())) {
            global = oldStaticAttribute->isGlobal();
        }
        else if (oldStaticAttribute->isGlobal() != global) {
            throw std::runtime_error("Conflicting variable linkage");
        }
        if (auto oldInitialValue = std::dynamic_pointer_cast<ConstantInitial>(
                oldStaticAttribute->getInitialValue())) {
            if (std::dynamic_pointer_cast<ConstantInitial>(initialValue)) {
                throw std::runtime_error(
                    "Conflicting file-scope variable definitions");
            }
            else {
                initialValue = oldInitialValue;
            }
        }
        else if (!std::dynamic_pointer_cast<ConstantInitial>(initialValue) &&
                 std::dynamic_pointer_cast<Tentative>(
                     oldStaticAttribute->getInitialValue())) {
            initialValue = std::make_shared<Tentative>();
        }
    }
    auto attribute = std::make_shared<StaticAttribute>(initialValue, global);
    symbols[declaration->getIdentifier()] =
        std::make_pair(std::make_shared<IntType>(), attribute);
}

void TypeCheckingPass::typeCheckLocalVariableDeclaration(
    std::shared_ptr<VariableDeclaration> declaration) {
    if (declaration->getOptStorageClass().has_value() &&
        std::dynamic_pointer_cast<ExternStorageClass>(
            declaration->getOptStorageClass().value())) {
        if (declaration->getOptInitializer().has_value()) {
            throw std::runtime_error(
                "Initializer on local extern variable declaration");
        }
        if (symbols.find(declaration->getIdentifier()) != symbols.end()) {
            auto oldDeclaration = symbols[declaration->getIdentifier()];
            auto oldType = oldDeclaration.first;
            if (*oldType != IntType()) {
                throw std::runtime_error("Function redeclared as variable");
            }
        }
        else {
            auto staticAttribute = std::make_shared<StaticAttribute>(
                std::make_shared<NoInitializer>(), true);
            symbols[declaration->getIdentifier()] =
                std::make_pair(std::make_shared<IntType>(), staticAttribute);
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
            initialValue = std::make_shared<ConstantInitial>(
                constantExpression->getValue());
        }
        else if (!declaration->getOptInitializer().has_value()) {
            initialValue = std::make_shared<ConstantInitial>(0);
        }
        else {
            throw std::runtime_error(
                "Non-constant initializer on local static variable");
        }
        auto staticAttribute =
            std::make_shared<StaticAttribute>(initialValue, false);
        symbols[declaration->getIdentifier()] =
            std::make_pair(std::make_shared<IntType>(), staticAttribute);
    }
    else {
        auto localAttribute = std::make_shared<LocalAttribute>();
        symbols[declaration->getIdentifier()] =
            std::make_pair(std::make_shared<IntType>(), localAttribute);
        if (declaration->getOptInitializer().has_value()) {
            typeCheckExpression(declaration->getOptInitializer().value());
        }
    }
}

void TypeCheckingPass::typeCheckBlock(std::shared_ptr<Block> block) {
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
                    throw std::runtime_error(
                        "Nested function definitions are not permitted");
                }
                if (functionDeclaration->getOptStorageClass().has_value() &&
                    std::dynamic_pointer_cast<StaticStorageClass>(
                        functionDeclaration->getOptStorageClass().value())) {
                    throw std::runtime_error(
                        "Static storage class on block-scope function "
                        "declaration");
                }
                typeCheckFunctionDeclaration(functionDeclaration);
            }
            else {
                throw std::runtime_error(
                    "Unsupported declaration type for type-checking");
            }
        }
        else if (auto sBlockItem =
                     std::dynamic_pointer_cast<SBlockItem>(blockItem)) {
            typeCheckStatement(sBlockItem->getStatement());
        }
        else {
            throw std::runtime_error(
                "Unsupported block item type for type-checking");
        }
    }
}

void TypeCheckingPass::typeCheckExpression(
    std::shared_ptr<Expression> expression) {
    if (auto functionCallExpression =
            std::dynamic_pointer_cast<FunctionCallExpression>(expression)) {
        auto fType = symbols[functionCallExpression->getIdentifier()].first;
        if (*fType == IntType()) {
            std::stringstream msg;
            msg << "Variable used as function name: "
                << functionCallExpression->getIdentifier();
            throw std::runtime_error(msg.str());
        }
        if (*fType !=
            FunctionType(functionCallExpression->getArguments()->size())) {
            std::stringstream msg;
            msg << "Function called with the wrong number of arguments: "
                << functionCallExpression->getIdentifier();
            throw std::runtime_error(msg.str());
        }
        for (auto &argument : *functionCallExpression->getArguments()) {
            typeCheckExpression(argument);
        }
    }
    else if (auto variableExpression =
                 std::dynamic_pointer_cast<VariableExpression>(expression)) {
        if (*(symbols[variableExpression->getIdentifier()].first) !=
            IntType()) {
            std::stringstream msg;
            msg << "Function name used as variable: "
                << variableExpression->getIdentifier();
            throw std::runtime_error(msg.str());
        }
    }
    else if (auto assignmentExpression =
                 std::dynamic_pointer_cast<AssignmentExpression>(expression)) {
        auto left = assignmentExpression->getLeft();
        auto right = assignmentExpression->getRight();
        typeCheckExpression(left);
        typeCheckExpression(right);
        // Note: For now, we only support assignments to int-type variables.
        if (*(symbols[std::dynamic_pointer_cast<VariableExpression>(left)
                          ->getIdentifier()]
                  .first) != IntType()) {
            std::stringstream msg;
            msg << "Assignment to a function name: "
                << std::dynamic_pointer_cast<VariableExpression>(left)
                       ->getIdentifier();
            throw std::runtime_error(msg.str());
        }
    }
    else if (auto unaryExpression =
                 std::dynamic_pointer_cast<UnaryExpression>(expression)) {
        typeCheckExpression(unaryExpression->getExpression());
    }
    else if (auto binaryExpression =
                 std::dynamic_pointer_cast<BinaryExpression>(expression)) {
        typeCheckExpression(binaryExpression->getLeft());
        typeCheckExpression(binaryExpression->getRight());
    }
    else if (auto conditionalExpression =
                 std::dynamic_pointer_cast<ConditionalExpression>(expression)) {
        typeCheckExpression(conditionalExpression->getCondition());
        typeCheckExpression(conditionalExpression->getThenExpression());
        typeCheckExpression(conditionalExpression->getElseExpression());
    }
}

void TypeCheckingPass::typeCheckStatement(
    std::shared_ptr<Statement> statement) {
    if (auto returnStatement =
            std::dynamic_pointer_cast<ReturnStatement>(statement)) {
        if (returnStatement->getExpression()) {
            typeCheckExpression(returnStatement->getExpression());
        }
    }
    else if (auto expressionStatement =
                 std::dynamic_pointer_cast<ExpressionStatement>(statement)) {
        typeCheckExpression(expressionStatement->getExpression());
    }
    else if (auto compoundStatement =
                 std::dynamic_pointer_cast<CompoundStatement>(statement)) {
        typeCheckBlock(compoundStatement->getBlock());
    }
    else if (auto whileStatement =
                 std::dynamic_pointer_cast<WhileStatement>(statement)) {
        typeCheckExpression(whileStatement->getCondition());
        typeCheckStatement(whileStatement->getBody());
    }
    else if (auto doWhileStatement =
                 std::dynamic_pointer_cast<DoWhileStatement>(statement)) {
        typeCheckExpression(doWhileStatement->getCondition());
        typeCheckStatement(doWhileStatement->getBody());
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
        typeCheckStatement(forStatement->getBody());
    }
    else if (auto ifStatement =
                 std::dynamic_pointer_cast<IfStatement>(statement)) {
        typeCheckExpression(ifStatement->getCondition());
        typeCheckStatement(ifStatement->getThenStatement());
        if (ifStatement->getElseOptStatement().has_value()) {
            typeCheckStatement(ifStatement->getElseOptStatement().value());
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
            throw std::runtime_error("Storage class in for-init declaration");
        }
        typeCheckLocalVariableDeclaration(initDecl->getVariableDeclaration());
    }
    else {
        throw std::runtime_error("Unsupported for-init type for type-checking");
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
            throw std::runtime_error(
                "Unsupported block item type for loop labeling");
        }
    }
    return std::make_shared<Block>(std::move(newBlockItems));
}
/*
 * End: Functions for the loop-labeling pass.
 */
} // namespace AST
