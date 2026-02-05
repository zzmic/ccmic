#include "semanticAnalysisPasses.h"
#include "block.h"
#include "blockItem.h"
#include "constant.h"
#include "declaration.h"
#include "expression.h"
#include "forInit.h"
#include "frontendSymbolTable.h"
#include "operator.h"
#include "program.h"
#include "statement.h"
#include "storageClass.h"
#include "type.h"
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

/**
 * Unnamed namespace for helper functions for the semantic analysis passes.
 */
namespace {
/**
 * Clone a type.
 *
 * @param type The type to clone.
 * @return The cloned type.
 */
std::unique_ptr<AST::Type> cloneType(const AST::Type *type) {
    if (!type) {
        return nullptr;
    }
    else if (dynamic_cast<const AST::IntType *>(type)) {
        return std::make_unique<AST::IntType>();
    }
    else if (dynamic_cast<const AST::LongType *>(type)) {
        return std::make_unique<AST::LongType>();
    }
    else if (dynamic_cast<const AST::UIntType *>(type)) {
        return std::make_unique<AST::UIntType>();
    }
    else if (dynamic_cast<const AST::ULongType *>(type)) {
        return std::make_unique<AST::ULongType>();
    }
    else if (auto functionType =
                 dynamic_cast<const AST::FunctionType *>(type)) {
        auto parameterTypes =
            std::make_unique<std::vector<std::unique_ptr<AST::Type>>>();
        parameterTypes->reserve(functionType->getParameterTypes().size());
        for (const auto &parameter : functionType->getParameterTypes()) {
            parameterTypes->emplace_back(cloneType(parameter.get()));
        }
        return std::make_unique<AST::FunctionType>(
            std::move(parameterTypes),
            cloneType(&functionType->getReturnType()));
    }
    const auto &r = *type;
    throw std::logic_error(
        "Unsupported type in cloneType in semanticAnalysisPasses: " +
        std::string(typeid(r).name()));
}

/**
 * Clone a constant.
 *
 * @param constant The constant to clone.
 * @return The cloned constant.
 */
std::unique_ptr<AST::Constant> cloneConstant(const AST::Constant *constant) {
    if (auto intConst = dynamic_cast<const AST::ConstantInt *>(constant)) {
        return std::make_unique<AST::ConstantInt>(intConst->getValue());
    }
    else if (auto longConst =
                 dynamic_cast<const AST::ConstantLong *>(constant)) {
        return std::make_unique<AST::ConstantLong>(longConst->getValue());
    }
    else if (auto uintConst =
                 dynamic_cast<const AST::ConstantUInt *>(constant)) {
        return std::make_unique<AST::ConstantUInt>(uintConst->getValue());
    }
    else if (auto ulongConst =
                 dynamic_cast<const AST::ConstantULong *>(constant)) {
        return std::make_unique<AST::ConstantULong>(ulongConst->getValue());
    }
    throw std::logic_error("Unsupported constant type in cloneConstant in "
                           "semanticAnalysisPasses");
}

/**
 * Clone a unary operator.
 *
 * @param op The unary operator to clone.
 * @return The cloned unary operator.
 */
std::unique_ptr<AST::UnaryOperator>
cloneUnaryOperator(const AST::UnaryOperator *op) {
    if (dynamic_cast<const AST::ComplementOperator *>(op)) {
        return std::make_unique<AST::ComplementOperator>();
    }
    else if (dynamic_cast<const AST::NegateOperator *>(op)) {
        return std::make_unique<AST::NegateOperator>();
    }
    else if (dynamic_cast<const AST::NotOperator *>(op)) {
        return std::make_unique<AST::NotOperator>();
    }
    throw std::logic_error("Unsupported unary operator in cloneUnaryOperator "
                           "in semanticAnalysisPasses");
}

/**
 * Clone a binary operator.
 *
 * @param op The binary operator to clone.
 * @return The cloned binary operator.
 */
std::unique_ptr<AST::BinaryOperator>
cloneBinaryOperator(const AST::BinaryOperator *op) {
    if (dynamic_cast<const AST::AddOperator *>(op)) {
        return std::make_unique<AST::AddOperator>();
    }
    else if (dynamic_cast<const AST::SubtractOperator *>(op)) {
        return std::make_unique<AST::SubtractOperator>();
    }
    else if (dynamic_cast<const AST::MultiplyOperator *>(op)) {
        return std::make_unique<AST::MultiplyOperator>();
    }
    else if (dynamic_cast<const AST::DivideOperator *>(op)) {
        return std::make_unique<AST::DivideOperator>();
    }
    else if (dynamic_cast<const AST::RemainderOperator *>(op)) {
        return std::make_unique<AST::RemainderOperator>();
    }
    else if (dynamic_cast<const AST::AndOperator *>(op)) {
        return std::make_unique<AST::AndOperator>();
    }
    else if (dynamic_cast<const AST::OrOperator *>(op)) {
        return std::make_unique<AST::OrOperator>();
    }
    else if (dynamic_cast<const AST::EqualOperator *>(op)) {
        return std::make_unique<AST::EqualOperator>();
    }
    else if (dynamic_cast<const AST::NotEqualOperator *>(op)) {
        return std::make_unique<AST::NotEqualOperator>();
    }
    else if (dynamic_cast<const AST::LessThanOperator *>(op)) {
        return std::make_unique<AST::LessThanOperator>();
    }
    else if (dynamic_cast<const AST::LessThanOrEqualOperator *>(op)) {
        return std::make_unique<AST::LessThanOrEqualOperator>();
    }
    else if (dynamic_cast<const AST::GreaterThanOperator *>(op)) {
        return std::make_unique<AST::GreaterThanOperator>();
    }
    else if (dynamic_cast<const AST::GreaterThanOrEqualOperator *>(op)) {
        return std::make_unique<AST::GreaterThanOrEqualOperator>();
    }
    else if (dynamic_cast<const AST::AssignmentOperator *>(op)) {
        return std::make_unique<AST::AssignmentOperator>();
    }
    throw std::logic_error("Unsupported binary operator in cloneBinaryOperator "
                           "in semanticAnalysisPasses");
}

/**
 * Clone an expression.
 *
 * @param expression The expression to clone.
 * @return The cloned expression.
 */
std::unique_ptr<AST::Expression>
cloneExpression(const AST::Expression *expression) {
    if (!expression) {
        return nullptr;
    }
    else if (auto assignmentExpression =
                 dynamic_cast<const AST::AssignmentExpression *>(expression)) {
        auto left = cloneExpression(assignmentExpression->getLeft());
        auto right = cloneExpression(assignmentExpression->getRight());
        auto cloned = std::make_unique<AST::AssignmentExpression>(
            std::move(left), std::move(right));
        cloned->setExpType(cloneType(assignmentExpression->getExpType()));
        return cloned;
    }
    else if (auto variableExpression =
                 dynamic_cast<const AST::VariableExpression *>(expression)) {
        auto cloned = std::make_unique<AST::VariableExpression>(
            variableExpression->getIdentifier(),
            cloneType(variableExpression->getExpType()));
        return cloned;
    }
    else if (auto constantExpression =
                 dynamic_cast<const AST::ConstantExpression *>(expression)) {
        auto cloned = std::make_unique<AST::ConstantExpression>(
            cloneConstant(constantExpression->getConstant()),
            cloneType(constantExpression->getExpType()));
        return cloned;
    }
    else if (auto castExpression =
                 dynamic_cast<const AST::CastExpression *>(expression)) {
        auto cloned = std::make_unique<AST::CastExpression>(
            cloneType(castExpression->getTargetType()),
            cloneExpression(castExpression->getExpression()),
            cloneType(castExpression->getExpType()));
        return cloned;
    }
    else if (auto unaryExpression =
                 dynamic_cast<const AST::UnaryExpression *>(expression)) {
        auto operand = cloneExpression(unaryExpression->getExpression());
        auto cloned = std::make_unique<AST::UnaryExpression>(
            cloneUnaryOperator(unaryExpression->getOperator()),
            std::move(operand), cloneType(unaryExpression->getExpType()));
        return cloned;
    }
    else if (auto binaryExpression =
                 dynamic_cast<const AST::BinaryExpression *>(expression)) {
        auto left = cloneExpression(binaryExpression->getLeft());
        auto right = cloneExpression(binaryExpression->getRight());
        auto cloned = std::make_unique<AST::BinaryExpression>(
            std::move(left),
            cloneBinaryOperator(binaryExpression->getOperator()),
            std::move(right), cloneType(binaryExpression->getExpType()));
        return cloned;
    }
    else if (auto conditionalExpression =
                 dynamic_cast<const AST::ConditionalExpression *>(expression)) {
        auto cloned = std::make_unique<AST::ConditionalExpression>(
            cloneExpression(conditionalExpression->getCondition()),
            cloneExpression(conditionalExpression->getThenExpression()),
            cloneExpression(conditionalExpression->getElseExpression()),
            cloneType(conditionalExpression->getExpType()));
        return cloned;
    }
    else if (auto functionCallExpression =
                 dynamic_cast<const AST::FunctionCallExpression *>(
                     expression)) {
        auto arguments =
            std::make_unique<std::vector<std::unique_ptr<AST::Expression>>>();
        arguments->reserve(functionCallExpression->getArguments().size());
        for (const auto &argument : functionCallExpression->getArguments()) {
            arguments->emplace_back(cloneExpression(argument.get()));
        }
        auto cloned = std::make_unique<AST::FunctionCallExpression>(
            functionCallExpression->getIdentifier(), std::move(arguments),
            cloneType(functionCallExpression->getExpType()));
        return cloned;
    }
    const auto &r = *expression;
    throw std::logic_error("Unsupported expression type in cloneExpression in "
                           "semanticAnalysisPasses: " +
                           std::string(typeid(r).name()));
}

/**
 * Clone an initial value.
 *
 * @param initialValue The initial value to clone.
 * @return The cloned initial value.
 */
std::unique_ptr<AST::InitialValue>
cloneInitialValue(const AST::InitialValue *initialValue) {
    if (dynamic_cast<const AST::NoInitializer *>(initialValue)) {
        return std::make_unique<AST::NoInitializer>();
    }
    else if (dynamic_cast<const AST::Tentative *>(initialValue)) {
        return std::make_unique<AST::Tentative>();
    }
    else if (auto initial = dynamic_cast<const AST::Initial *>(initialValue)) {
        auto value = initial->getStaticInit()->getValue();
        if (std::holds_alternative<int>(value)) {
            return std::make_unique<AST::Initial>(std::get<int>(value));
        }
        else if (std::holds_alternative<long>(value)) {
            return std::make_unique<AST::Initial>(std::get<long>(value));
        }
        else if (std::holds_alternative<unsigned int>(value)) {
            return std::make_unique<AST::Initial>(
                std::get<unsigned int>(value));
        }
        else if (std::holds_alternative<unsigned long>(value)) {
            return std::make_unique<AST::Initial>(
                std::get<unsigned long>(value));
        }
        else {
            const auto &r = *initialValue;
            throw std::logic_error(
                "Unsupported initial value type in cloneInitialValue in "
                "semanticAnalysisPasses: " +
                std::string(typeid(r).name()));
        }
    }
    throw std::logic_error("Unsupported initial value type in "
                           "cloneInitialValue in semanticAnalysisPasses");
}

/**
 * Get the size of a type in bytes.
 *
 * For X86-64 System V ABI:
 * - `int` and `unsigned int` are 4 bytes (32-bit).
 * - `long` and `unsigned long` are 8 bytes (64-bit).
 *
 * @param type The type to get the size of.
 * @return The size of the type in bytes.
 */
int getTypeSize(const AST::Type *type) {
    if (dynamic_cast<const AST::IntType *>(type) ||
        dynamic_cast<const AST::UIntType *>(type)) {
        return 4;
    }
    else if (dynamic_cast<const AST::LongType *>(type) ||
             dynamic_cast<const AST::ULongType *>(type)) {
        return 8;
    }
    const auto &r = *type;
    throw std::logic_error(
        "Unsupported type in getTypeSize in semanticAnalysisPasses: " +
        std::string(typeid(r).name()));
}

/**
 * Check if a type is a signed type (`int` or `long`).
 *
 * @param type The type to check.
 * @return True if the type is a signed type (`int` or `long`), false otherwise.
 */
bool isSigned(const AST::Type *type) {
    return dynamic_cast<const AST::IntType *>(type) ||
           dynamic_cast<const AST::LongType *>(type);
}

/**
 * Check if a type is an arithmetic type (`int`, `long`, `unsigned int`, or
 * `unsigned long`).
 *
 * @param type The type to check.
 * @return True if the type is an arithmetic type (`int`, `long`, `unsigned
 * int`, or `unsigned long`), false otherwise.
 */
bool isArithmeticType(const AST::Type *type) {
    return dynamic_cast<const AST::IntType *>(type) ||
           dynamic_cast<const AST::LongType *>(type) ||
           dynamic_cast<const AST::UIntType *>(type) ||
           dynamic_cast<const AST::ULongType *>(type);
}
} // namespace

namespace AST {
/*
 * Start: Functions for the identifier-resolution pass.
 */
int IdentifierResolutionPass::resolveProgram(Program &program) {
    // Initialize an empty identifier map (that will be passed to the helpers).
    // Instead of maintaining a "global" identifier map, we pass the identifier
    // map to the helper functions to, together with `copyIdentifierMap`, ensure
    // that each identifier map is corresponding to a specific block.
    auto identifierMap = std::unordered_map<std::string, MapEntry>();

    // At the top level, resolve the list of declarations in the
    // program.
    for (auto &declaration : program.getDeclarations()) {
        if (auto *functionDeclaration =
                dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            resolveFunctionDeclaration(functionDeclaration, identifierMap);
        }
        else if (auto *variableDeclaration =
                     dynamic_cast<VariableDeclaration *>(declaration.get())) {
            resolveFileScopeVariableDeclaration(variableDeclaration,
                                                identifierMap);
        }
        else {
            const auto &r = *declaration;
            throw std::logic_error(
                "Unsupported declaration type for identifier resolution in "
                "resolveProgram in IdentifierResolutionPass: " +
                std::string(typeid(r).name()));
        }
    }

    return variableResolutionCounter;
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
    return identifier + "." + std::to_string(variableResolutionCounter++);
}

void IdentifierResolutionPass::resolveFileScopeVariableDeclaration(
    VariableDeclaration *declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    identifierMap[declaration->getIdentifier()] =
        MapEntry(declaration->getIdentifier(), true, true);
}

void IdentifierResolutionPass::resolveLocalVariableDeclaration(
    VariableDeclaration *declaration,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (identifierMap.find(declaration->getIdentifier()) !=
        identifierMap.end()) {
        auto previousEntry = identifierMap[declaration->getIdentifier()];
        if (previousEntry.fromCurrentScopeOrNot()) {
            if (!(previousEntry.hasLinkageOrNot() &&
                  declaration->getOptStorageClass() &&
                  dynamic_cast<ExternStorageClass *>(
                      declaration->getOptStorageClass()))) {
                std::stringstream msg;
                msg << "Conflicting local variable declaration: "
                    << declaration->getIdentifier();
                throw std::logic_error(msg.str());
            }
        }
    }
    if (declaration->getOptStorageClass() &&
        dynamic_cast<ExternStorageClass *>(declaration->getOptStorageClass())) {
        identifierMap[declaration->getIdentifier()] =
            MapEntry(declaration->getIdentifier(), true, true);
    }
    else {
        auto declarationIdentifier = declaration->getIdentifier();
        auto uniqueVariableName =
            generateUniqueVariableName(declarationIdentifier);
        identifierMap[declarationIdentifier] =
            MapEntry(uniqueVariableName, true, false);
        auto optInitializer = declaration->getOptInitializer();
        if (optInitializer) {
            resolveExpression(optInitializer, identifierMap);
        }
        declaration->setIdentifier(
            identifierMap[declarationIdentifier].getNewName());
    }
}

void IdentifierResolutionPass::resolveStatement(
    Statement *statement,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto returnStatement = dynamic_cast<ReturnStatement *>(statement)) {
        // If the statement is a return statement, resolve the expression in the
        // return statement.
        resolveExpression(returnStatement->getExpression(), identifierMap);
    }
    else if (auto expressionStatement =
                 dynamic_cast<ExpressionStatement *>(statement)) {
        // If the statement is an expression statement, resolve the expression
        // in the expression statement.
        resolveExpression(expressionStatement->getExpression(), identifierMap);
    }
    else if (auto compoundStatement =
                 dynamic_cast<CompoundStatement *>(statement)) {
        // Copy the identifier map (with modifications) and resolve the block in
        // the compound statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        resolveBlock(compoundStatement->getBlock(), copiedIdentifierMap);
    }
    else if (auto whileStatement = dynamic_cast<WhileStatement *>(statement)) {
        // If the statement is a while-statement, resolve the condition
        // expression and the body statement in the while-statement.
        resolveExpression(whileStatement->getCondition(), identifierMap);
        resolveStatement(whileStatement->getBody(), identifierMap);
    }
    else if (auto doWhileStatement =
                 dynamic_cast<DoWhileStatement *>(statement)) {
        // If the statement is a do-while-statement, resolve the condition
        // expression and the body statement in the do-while-statement.
        resolveExpression(doWhileStatement->getCondition(), identifierMap);
        resolveStatement(doWhileStatement->getBody(), identifierMap);
    }
    else if (auto forStatement = dynamic_cast<ForStatement *>(statement)) {
        // Copy the identifier map (with modifications) and resolve the
        // for-init, (optional) condition, (optional) post, and body in the
        // for-statement.
        auto copiedIdentifierMap = copyIdentifierMap(identifierMap);
        resolveForInit(forStatement->getForInit(), copiedIdentifierMap);
        if (forStatement->getOptCondition()) {
            resolveExpression(forStatement->getOptCondition(),
                              copiedIdentifierMap);
        }
        if (forStatement->getOptPost()) {
            resolveExpression(forStatement->getOptPost(), copiedIdentifierMap);
        }
        resolveStatement(forStatement->getBody(), copiedIdentifierMap);
    }
    else if (auto ifStatement = dynamic_cast<IfStatement *>(statement)) {
        // If the statement is an if-statement, resolve the condition
        // expression, then-statement, and (optional) else-statement in the
        // if-statement.
        resolveExpression(ifStatement->getCondition(), identifierMap);
        resolveStatement(ifStatement->getThenStatement(), identifierMap);
        if (ifStatement->getElseOptStatement()) {
            resolveStatement(ifStatement->getElseOptStatement(), identifierMap);
        }
    }
    else if ((dynamic_cast<BreakStatement *>(statement)) ||
             (dynamic_cast<ContinueStatement *>(statement)) ||
             (dynamic_cast<NullStatement *>(statement))) {
        // If the statement is a break statement, continue statement, or null
        // statement, do nothing.
    }
    else {
        throw std::logic_error(
            "Unsupported statement type for identifier resolution in "
            "resolveStatement in IdentifierResolutionPass");
    }
}

void IdentifierResolutionPass::resolveExpression(
    Expression *expression,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    if (auto assignmentExpression =
            dynamic_cast<AssignmentExpression *>(expression)) {
        if (!(dynamic_cast<VariableExpression *>(
                assignmentExpression->getLeft()))) {
            const auto &r = *assignmentExpression->getLeft();
            throw std::logic_error(
                "Invalid lvalue in assignment expression in resolveExpression "
                "in IdentifierResolutionPass: " +
                std::string(typeid(r).name()));
        }
        resolveExpression(assignmentExpression->getLeft(), identifierMap);
        resolveExpression(assignmentExpression->getRight(), identifierMap);
    }
    else if (auto variableExpression =
                 dynamic_cast<VariableExpression *>(expression)) {
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
        variableExpression->setIdentifier(
            identifierMap[identifier].getNewName());
    }
    else if (auto unaryExpression =
                 dynamic_cast<UnaryExpression *>(expression)) {
        // If the expression is a unary expression, resolve the expression in
        // the unary expression.
        resolveExpression(unaryExpression->getExpression(), identifierMap);
    }
    else if (auto binaryExpression =
                 dynamic_cast<BinaryExpression *>(expression)) {
        // If the expression is a binary expression, resolve the left and right
        // expressions in the binary expression.
        resolveExpression(binaryExpression->getLeft(), identifierMap);
        resolveExpression(binaryExpression->getRight(), identifierMap);
    }
    else if (auto conditionalExpression =
                 dynamic_cast<ConditionalExpression *>(expression)) {
        // If the expression is a conditional expression, resolve the condition
        // expression, then-expression, and else-expression in the conditional
        // expression.
        resolveExpression(conditionalExpression->getCondition(), identifierMap);
        resolveExpression(conditionalExpression->getThenExpression(),
                          identifierMap);
        resolveExpression(conditionalExpression->getElseExpression(),
                          identifierMap);
    }
    else if (auto functionCallExpression =
                 dynamic_cast<FunctionCallExpression *>(expression)) {
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
        functionCallExpression->setIdentifier(resolvedFunctionName);
        for (auto &argument : functionCallExpression->getArguments()) {
            resolveExpression(argument.get(), identifierMap);
        }
    }
    else if (auto castExpression = dynamic_cast<CastExpression *>(expression)) {
        resolveExpression(castExpression->getExpression(), identifierMap);
    }
    else if (dynamic_cast<ConstantExpression *>(expression)) {
    }
    else {
        throw std::logic_error(
            "Unsupported expression type for identifier resolution in "
            "resolveExpression in IdentifierResolutionPass");
    }
}

void IdentifierResolutionPass::resolveBlock(
    Block *block, std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Get the block items from the block and resolve the variables in each
    // block item.
    for (auto &blockItem : block->getBlockItems()) {
        if (auto dBlockItem = dynamic_cast<DBlockItem *>(blockItem.get())) {
            if (auto variableDeclaration = dynamic_cast<VariableDeclaration *>(
                    dBlockItem->getDeclaration())) {
                resolveLocalVariableDeclaration(variableDeclaration,
                                                identifierMap);
            }
            else if (auto functionDeclaration =
                         dynamic_cast<FunctionDeclaration *>(
                             dBlockItem->getDeclaration())) {
                resolveFunctionDeclaration(functionDeclaration, identifierMap);
            }
            else {
                const auto &r = *dBlockItem->getDeclaration();
                throw std::logic_error(
                    "Unsupported declaration type for identifier resolution in "
                    "resolveBlock in IdentifierResolutionPass: " +
                    std::string(typeid(r).name()));
            }
        }
        else if (auto sBlockItem =
                     dynamic_cast<SBlockItem *>(blockItem.get())) {
            resolveStatement(sBlockItem->getStatement(), identifierMap);
        }
        else {
            const auto &r = *blockItem;
            throw std::logic_error(
                "Unsupported block item typen for identifier resolution in "
                "resolveBlock in IdentifierResolutionPass: " +
                std::string(typeid(r).name()));
        }
    }
}

void IdentifierResolutionPass::resolveForInit(
    ForInit *forInit,
    std::unordered_map<std::string, MapEntry> &identifierMap) {
    // Resolve the for-init based on the type of the for init.
    if (auto initExpr = dynamic_cast<InitExpr *>(forInit)) {
        if (auto *expr = initExpr->getExpression()) {
            resolveExpression(expr, identifierMap);
        }
    }
    else if (auto initDecl = dynamic_cast<InitDecl *>(forInit)) {
        resolveLocalVariableDeclaration(initDecl->getVariableDeclaration(),
                                        identifierMap);
    }
    else {
        throw std::logic_error(
            "Unsupported for-init type for identifier resolution in "
            "resolveForInit in IdentifierResolutionPass");
    }
}

void IdentifierResolutionPass::resolveFunctionDeclaration(
    FunctionDeclaration *declaration,
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
    auto resolvedParameters = std::make_unique<std::vector<std::string>>();
    for (const auto &parameter : declaration->getParameterIdentifiers()) {
        // Skip the built-in types `int` and `long` since they are not actual
        // parameters.
        if (parameter == "int" || parameter == "long") {
            continue;
        }
        resolvedParameters->emplace_back(
            resolveParameter(parameter, innerIdentifierMap));
    }
    declaration->setParameters(std::move(resolvedParameters));
    if (declaration->getOptBody()) {
        resolveBlock(declaration->getOptBody(), innerIdentifierMap);
    }
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
TypeCheckingPass::TypeCheckingPass(FrontendSymbolTable &frontendSymbolTable)
    : frontendSymbolTable(frontendSymbolTable) {}

void TypeCheckingPass::typeCheckProgram(Program &program) {
    // Clear the symbol table for this compilation.
    frontendSymbolTable.clear();

    // Type-check the program.
    for (auto &declaration : program.getDeclarations()) {
        if (auto *functionDeclaration =
                dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            typeCheckFunctionDeclaration(functionDeclaration);
        }
        else if (auto *variableDeclaration =
                     dynamic_cast<VariableDeclaration *>(declaration.get())) {
            typeCheckFileScopeVariableDeclaration(variableDeclaration);
        }
        else {
            const auto &r = *declaration;
            throw std::logic_error(
                "Unsupported declaration type for type checking at top level "
                "in typeCheckProgram in TypeCheckingPass: " +
                std::string(typeid(r).name()));
        }
    }
}

std::unique_ptr<StaticInit> TypeCheckingPass::convertStaticConstantToStaticInit(
    const Type *varType, const ConstantExpression *constantExpression) {
    // Extract the numeric value from the AST constant.
    auto variantValue = constantExpression->getConstantInVariant();

    // Convert to `unsigned long` to preserve bit patterns for all types,
    // allowing uniform handling regardless of the source type.
    unsigned long numericValue = 0;
    if (std::holds_alternative<int>(variantValue)) {
        numericValue = static_cast<unsigned int>(std::get<int>(variantValue));
    }
    else if (std::holds_alternative<long>(variantValue)) {
        numericValue = static_cast<unsigned long>(std::get<long>(variantValue));
    }
    else if (std::holds_alternative<unsigned int>(variantValue)) {
        numericValue = std::get<unsigned int>(variantValue);
    }
    else if (std::holds_alternative<unsigned long>(variantValue)) {
        numericValue = std::get<unsigned long>(variantValue);
    }
    else {
        const auto &r = *constantExpression;
        throw std::logic_error(
            "Unsupported constant type in convertStaticConstantToStaticInit in "
            "semanticAnalysisPasses: " +
            std::string(typeid(r).name()));
    }

    if (*varType == IntType()) {
        return std::make_unique<IntInit>(static_cast<int>(numericValue));
    }
    else if (*varType == LongType()) {
        return std::make_unique<LongInit>(static_cast<long>(numericValue));
    }
    else if (*varType == UIntType()) {
        return std::make_unique<UIntInit>(
            static_cast<unsigned int>(numericValue));
    }
    else if (*varType == ULongType()) {
        return std::make_unique<ULongInit>(
            static_cast<unsigned long>(numericValue));
    }
    else {
        const auto &r = *varType;
        throw std::logic_error("Unsupported type in static initializer in "
                               "semanticAnalysisPasses: " +
                               std::string(typeid(r).name()));
    }
}

std::unique_ptr<Type> TypeCheckingPass::getCommonType(const Type *type1,
                                                      const Type *type2) {
    // If `type1` is `nullptr`, throw an error.
    if (!type1) {
        throw std::logic_error(
            "Null type1 in getCommonType in semanticAnalysisPasses");
    }
    // If `type2` is `nullptr`, return `type1`.
    if (!type2) {
        return cloneType(type1);
    }
    // Rule 1: If both types are the same, return `type1` (or `type2`).
    if (*type1 == *type2) {
        return cloneType(type1);
    }
    int size1 = getTypeSize(type1);
    int size2 = getTypeSize(type2);
    // Rule 2: If both types are the same size, return the unsigned one.
    if (size1 == size2) {
        if (isSigned(type1)) {
            return cloneType(type2);
        }
        return cloneType(type1);
    }
    // Rule 3: If the types have different sizes, return the larger one.
    if (size1 > size2) {
        return cloneType(type1);
    }
    return cloneType(type2);
}

std::unique_ptr<Expression>
TypeCheckingPass::convertTo(const Expression *expression,
                            const Type *targetType) {
    if (!expression) {
        throw std::logic_error(
            "Null expression in convertTo in semanticAnalysisPasses");
    }
    if (!targetType) {
        throw std::logic_error(
            "Null target type in convertTo in semanticAnalysisPasses");
    }
    // Otherwise, wrap the expression in a cast expression and annotate the
    // result with the correct type.
    if (expression->getExpType() && *expression->getExpType() == *targetType) {
        return cloneExpression(expression);
    }
    auto castExpression = std::make_unique<CastExpression>(
        cloneType(targetType), cloneExpression(expression));
    castExpression->setExpType(cloneType(targetType));
    return castExpression;
}

void TypeCheckingPass::typeCheckFunctionDeclaration(
    FunctionDeclaration *declaration) {
    auto funType = declaration->getFunType();
    auto funTypePtr = dynamic_cast<FunctionType *>(funType);
    if (!funTypePtr) {
        throw std::logic_error(
            "Function type is not a FunctionType in "
            "typeCheckFunctionDeclaration in TypeCheckingPass");
    }
    auto hasBody = (declaration->getOptBody() != nullptr);
    auto alreadyDefined = false;
    auto global = true;

    if (declaration->getOptStorageClass() &&
        dynamic_cast<StaticStorageClass *>(declaration->getOptStorageClass())) {
        global = false;
    }
    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto &oldDeclaration =
            frontendSymbolTable[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first.get();
        if (*oldType != *funType) {
            const auto &r = *oldType;
            const auto &r2 = *funType;
            throw std::logic_error(
                "Incompatible function declarations in "
                "typeCheckFunctionDeclaration in TypeCheckingPass: " +
                std::string(typeid(r).name()) + " and " +
                std::string(typeid(r2).name()));
        }
        auto *oldFunctionAttribute =
            dynamic_cast<FunctionAttribute *>(oldDeclaration.second.get());
        alreadyDefined = oldFunctionAttribute->isDefined();
        if (alreadyDefined && hasBody) {
            throw std::logic_error(
                "Function redefinition in typeCheckFunctionDeclaration in "
                "TypeCheckingPass: " +
                declaration->getIdentifier());
        }
        if (oldFunctionAttribute->isGlobal() &&
            declaration->getOptStorageClass() &&
            dynamic_cast<StaticStorageClass *>(
                declaration->getOptStorageClass())) {
            throw std::logic_error(
                "Static function declaration follows non-static in "
                "typeCheckFunctionDeclaration in TypeCheckingPass");
        }
        global = oldFunctionAttribute->isGlobal();
    }

    auto attribute =
        std::make_unique<FunctionAttribute>(alreadyDefined || hasBody, global);
    frontendSymbolTable[declaration->getIdentifier()] = {cloneType(funType),
                                                         std::move(attribute)};

    if (hasBody) {
        const auto &funcParameterTypes = funTypePtr->getParameterTypes();
        auto &parameterIdentifiers = declaration->getParameterIdentifiers();
        // Set parameter types in the symbol table based on the function's
        // parameter types.
        for (size_t i = 0; i < parameterIdentifiers.size(); ++i) {
            // If the parameter type is available, use it.
            if (i < funcParameterTypes.size()) {
                frontendSymbolTable[parameterIdentifiers[i]] = {
                    cloneType(funcParameterTypes[i].get()),
                    std::make_unique<LocalAttribute>()};
            }
            else {
                // Otherwise, fallback to `IntType`.
                frontendSymbolTable[parameterIdentifiers[i]] = {
                    std::make_unique<IntType>(),
                    std::make_unique<LocalAttribute>()};
            }
        }
        // Provide the enclosing function's name for the later type-checking of
        // the return statement.
        typeCheckBlock(declaration->getOptBody(), declaration->getIdentifier());
    }
}

void TypeCheckingPass::typeCheckFileScopeVariableDeclaration(
    VariableDeclaration *declaration) {
    auto varType = declaration->getVarType();
    if (!isArithmeticType(varType)) {
        const auto &r = *varType;
        throw std::logic_error(
            "Unsupported variable type for file-scope variables in "
            "typeCheckFileScopeVariableDeclaration in TypeCheckingPass: " +
            std::string(typeid(r).name()));
    }

    auto initialValue = std::make_unique<InitialValue>();

    if (declaration->getOptInitializer() &&
        dynamic_cast<ConstantExpression *>(declaration->getOptInitializer())) {
        auto constantExpression = dynamic_cast<ConstantExpression *>(
            declaration->getOptInitializer());
        auto variantValue = constantExpression->getConstantInVariant();

        // Convert to `unsigned long` to preserve bit patterns for all types,
        // allowing uniform handling regardless of the source type.
        unsigned long numericValue = 0;
        if (std::holds_alternative<int>(variantValue)) {
            numericValue = static_cast<unsigned long>(
                static_cast<long>(std::get<int>(variantValue)));
        }
        else if (std::holds_alternative<long>(variantValue)) {
            numericValue =
                static_cast<unsigned long>(std::get<long>(variantValue));
        }
        else if (std::holds_alternative<unsigned int>(variantValue)) {
            numericValue = std::get<unsigned int>(variantValue);
        }
        else if (std::holds_alternative<unsigned long>(variantValue)) {
            numericValue = std::get<unsigned long>(variantValue);
        }
        else {
            throw std::logic_error(
                "Unsupported constant type in static initializer in "
                "typeCheckFileScopeVariableDeclaration in TypeCheckingPass: " +
                std::string(typeid(variantValue).name()));
        }

        // Create the initial value with the appropriate target type.
        if (dynamic_cast<IntType *>(varType)) {
            initialValue =
                std::make_unique<Initial>(static_cast<int>(numericValue));
        }
        else if (dynamic_cast<LongType *>(varType)) {
            initialValue =
                std::make_unique<Initial>(static_cast<long>(numericValue));
        }
        else if (dynamic_cast<UIntType *>(varType)) {
            initialValue = std::make_unique<Initial>(
                static_cast<unsigned int>(numericValue));
        }
        else if (dynamic_cast<ULongType *>(varType)) {
            initialValue = std::make_unique<Initial>(
                static_cast<unsigned long>(numericValue));
        }
        else {
            const auto &r = *varType;
            throw std::logic_error(
                "Unsupported type in static initializer in "
                "typeCheckFileScopeVariableDeclaration in TypeCheckingPass: " +
                std::string(typeid(r).name()));
        }
    }
    else if (!declaration->getOptInitializer()) {
        if (declaration->getOptStorageClass() &&
            dynamic_cast<ExternStorageClass *>(
                declaration->getOptStorageClass())) {
            initialValue = std::make_unique<NoInitializer>();
        }
        else {
            initialValue = std::make_unique<Tentative>();
        }
    }
    else {
        throw std::logic_error(
            "Non-constant initializer in typeCheckFileScopeVariableDeclaration "
            "in TypeCheckingPass!");
    }

    // Determine the linkage of the variable.
    auto global = (!declaration->getOptStorageClass()) ||
                  (declaration->getOptStorageClass() &&
                   !(dynamic_cast<StaticStorageClass *>(
                       declaration->getOptStorageClass())));

    if (frontendSymbolTable.find(declaration->getIdentifier()) !=
        frontendSymbolTable.end()) {
        auto &oldDeclaration =
            frontendSymbolTable[declaration->getIdentifier()];
        auto oldType = oldDeclaration.first.get();
        if (*oldType != *varType) {
            throw std::logic_error(
                "Function redeclared as variable in "
                "typeCheckFileScopeVariableDeclaration in TypeCheckingPass");
        }
        auto *oldStaticAttribute =
            dynamic_cast<StaticAttribute *>(oldDeclaration.second.get());
        if (declaration->getOptStorageClass() &&
            dynamic_cast<ExternStorageClass *>(
                declaration->getOptStorageClass())) {
            global = oldStaticAttribute->isGlobal();
        }
        else if (oldStaticAttribute->isGlobal() != global) {
            throw std::logic_error(
                "Conflicting variable linkage in "
                "typeCheckFileScopeVariableDeclaration in TypeCheckingPass");
        }
        if (dynamic_cast<Initial *>(oldStaticAttribute->getInitialValue())) {
            if (dynamic_cast<Initial *>(initialValue.get())) {
                throw std::logic_error(
                    "Conflicting file-scope variable definitions in "
                    "typeCheckFileScopeVariableDeclaration in "
                    "TypeCheckingPass");
            }
            initialValue =
                cloneInitialValue(oldStaticAttribute->getInitialValue());
        }
        else if (!dynamic_cast<Initial *>(initialValue.get()) &&
                 dynamic_cast<Tentative *>(
                     oldStaticAttribute->getInitialValue())) {
            initialValue = std::make_unique<Tentative>();
        }
    }

    auto attribute =
        std::make_unique<StaticAttribute>(std::move(initialValue), global);
    // Store the corresponding variable type and attribute in the symbol table.
    frontendSymbolTable[declaration->getIdentifier()] = {cloneType(varType),
                                                         std::move(attribute)};
}

void TypeCheckingPass::typeCheckLocalVariableDeclaration(
    VariableDeclaration *declaration) {
    auto varType = declaration->getVarType();
    if (!isArithmeticType(varType)) {
        throw std::logic_error(
            "Unsupported variable type for local variables in "
            "typeCheckLocalVariableDeclaration in TypeCheckingPass: " +
            std::string(typeid(varType).name()));
    }

    if (declaration->getOptStorageClass() &&
        dynamic_cast<ExternStorageClass *>(declaration->getOptStorageClass())) {
        if (declaration->getOptInitializer()) {
            throw std::logic_error(
                "Initializer on local extern variable declaration in "
                "typeCheckLocalVariableDeclaration in TypeCheckingPass");
        }
        if (frontendSymbolTable.find(declaration->getIdentifier()) !=
            frontendSymbolTable.end()) {
            auto &oldDeclaration =
                frontendSymbolTable[declaration->getIdentifier()];
            auto oldType = oldDeclaration.first.get();
            if (*oldType != *varType) {
                throw std::logic_error(
                    "Function redeclared as variable in "
                    "typeCheckLocalVariableDeclaration in TypeCheckingPass");
            }
        }
        else {
            auto staticAttribute = std::make_unique<StaticAttribute>(
                std::make_unique<NoInitializer>(), true);
            frontendSymbolTable[declaration->getIdentifier()] =
                std::make_pair(cloneType(varType), std::move(staticAttribute));
        }
    }
    else if (declaration->getOptStorageClass() &&
             dynamic_cast<StaticStorageClass *>(
                 declaration->getOptStorageClass())) {
        auto initialValue = std::make_unique<InitialValue>();
        if (declaration->getOptInitializer() &&
            dynamic_cast<ConstantExpression *>(
                declaration->getOptInitializer())) {
            auto constantExpression = dynamic_cast<ConstantExpression *>(
                declaration->getOptInitializer());
            auto variantValue = constantExpression->getConstantInVariant();

            // Convert to `unsigned long` to preserve bit patterns for all
            // types, allowing uniform handling regardless of the source type.
            unsigned long numericValue = 0;
            if (std::holds_alternative<int>(variantValue)) {
                numericValue = static_cast<unsigned long>(
                    static_cast<long>(std::get<int>(variantValue)));
            }
            else if (std::holds_alternative<long>(variantValue)) {
                numericValue =
                    static_cast<unsigned long>(std::get<long>(variantValue));
            }
            else if (std::holds_alternative<unsigned int>(variantValue)) {
                numericValue = std::get<unsigned int>(variantValue);
            }
            else if (std::holds_alternative<unsigned long>(variantValue)) {
                numericValue = std::get<unsigned long>(variantValue);
            }
            else {
                throw std::logic_error(
                    "Unsupported constant type in static initializer in "
                    "typeCheckLocalVariableDeclaration in TypeCheckingPass: " +
                    std::string(typeid(variantValue).name()));
            }

            // Create the initial value with the appropriate target type.
            if (dynamic_cast<IntType *>(varType)) {
                initialValue =
                    std::make_unique<Initial>(static_cast<int>(numericValue));
            }
            else if (dynamic_cast<LongType *>(varType)) {
                initialValue =
                    std::make_unique<Initial>(static_cast<long>(numericValue));
            }
            else if (dynamic_cast<UIntType *>(varType)) {
                initialValue = std::make_unique<Initial>(
                    static_cast<unsigned int>(numericValue));
            }
            else if (dynamic_cast<ULongType *>(varType)) {
                initialValue = std::make_unique<Initial>(
                    static_cast<unsigned long>(numericValue));
            }
            else {
                const auto &r = *varType;
                throw std::logic_error(
                    "Unsupported type in static initializer in "
                    "typeCheckLocalVariableDeclaration in TypeCheckingPass: " +
                    std::string(typeid(r).name()));
            }
        }
        else if (!declaration->getOptInitializer()) {
            initialValue = std::make_unique<Initial>(0);
        }
        else {
            throw std::logic_error(
                "Non-constant initializer on local static variable in "
                "typeCheckLocalVariableDeclaration in TypeCheckingPass");
        }
        auto staticAttribute =
            std::make_unique<StaticAttribute>(std::move(initialValue), false);
        frontendSymbolTable[declaration->getIdentifier()] =
            std::make_pair(cloneType(varType), std::move(staticAttribute));
    }
    else {
        auto localAttribute = std::make_unique<LocalAttribute>();
        frontendSymbolTable[declaration->getIdentifier()] =
            std::make_pair(cloneType(varType), std::move(localAttribute));
        if (declaration->getOptInitializer()) {
            auto *initializer = declaration->getOptInitializer();
            typeCheckExpression(initializer);
            declaration->setOptInitializer(convertTo(initializer, varType));
        }
    }
}

void TypeCheckingPass::typeCheckBlock(Block *block,
                                      std::string enclosingFunctionIdentifier) {
    for (auto &blockItem : block->getBlockItems()) {
        if (auto dBlockItem = dynamic_cast<DBlockItem *>(blockItem.get())) {
            if (auto variableDeclaration = dynamic_cast<VariableDeclaration *>(
                    dBlockItem->getDeclaration())) {
                typeCheckLocalVariableDeclaration(variableDeclaration);
            }
            else if (auto functionDeclaration =
                         dynamic_cast<FunctionDeclaration *>(
                             dBlockItem->getDeclaration())) {
                if (functionDeclaration->getOptBody()) {
                    throw std::logic_error(
                        "Nested function definitions are not permitted in "
                        "typeCheckBlock in TypeCheckingPass");
                }
                if (functionDeclaration->getOptStorageClass() &&
                    dynamic_cast<StaticStorageClass *>(
                        functionDeclaration->getOptStorageClass())) {
                    throw std::logic_error(
                        "Static storage class on block-scope function "
                        "declaration in typeCheckBlock in TypeCheckingPass");
                }
                typeCheckFunctionDeclaration(functionDeclaration);
            }
            else {
                const auto &r = *dBlockItem->getDeclaration();
                throw std::logic_error(
                    "Unsupported declaration type for type-checking in "
                    "typeCheckBlock in TypeCheckingPass: " +
                    std::string(typeid(r).name()));
            }
        }
        else if (auto sBlockItem =
                     dynamic_cast<SBlockItem *>(blockItem.get())) {
            // Provide the enclosing function's name for the later type-checking
            // of the return statement.
            typeCheckStatement(sBlockItem->getStatement(),
                               enclosingFunctionIdentifier);
        }
        else {
            const auto &r = *blockItem;
            throw std::logic_error(
                "Unsupported block item type for type-checking in "
                "typeCheckBlock in TypeCheckingPass: " +
                std::string(typeid(r).name()));
        }
    }
}

void TypeCheckingPass::typeCheckExpression(Expression *expression) {
    if (auto functionCallExpression =
            dynamic_cast<FunctionCallExpression *>(expression)) {
        auto fType =
            frontendSymbolTable[functionCallExpression->getIdentifier()]
                .first.get();
        if (isArithmeticType(fType)) {
            throw std::logic_error("Function name used as variable in "
                                   "typeCheckExpression in TypeCheckingPass: " +
                                   functionCallExpression->getIdentifier());
        }
        else {
            auto functionType = dynamic_cast<FunctionType *>(fType);
            const auto &parameterTypes = functionType->getParameterTypes();
            auto &arguments = functionCallExpression->getArguments();
            if (parameterTypes.size() != arguments.size()) {
                throw std::logic_error(
                    "Function called with a wrong number of arguments in "
                    "typeCheckExpression in TypeCheckingPass: " +
                    functionCallExpression->getIdentifier() + " expected " +
                    std::to_string(parameterTypes.size()) +
                    " arguments but got " + std::to_string(arguments.size()));
            }
            auto convertedArguments =
                std::make_unique<std::vector<std::unique_ptr<Expression>>>();
            convertedArguments->reserve(arguments.size());
            // Iterate over the function's arguments and parameters together.
            // Type-check each argument and convert it to the corresponding
            // parameter type.
            for (size_t i = 0; i < arguments.size(); ++i) {
                auto *argument = arguments[i].get();
                auto *parameterType = parameterTypes[i].get();
                typeCheckExpression(argument);
                convertedArguments->emplace_back(
                    convertTo(argument, parameterType));
            }
            // Set the function call expression's arguments to the converted
            // arguments.
            functionCallExpression->setArguments(std::move(convertedArguments));
            // Set the function call expression's type to the function's return
            // type.
            functionCallExpression->setExpType(
                cloneType(&functionType->getReturnType()));
        }
    }
    else if (auto constantExpression =
                 dynamic_cast<ConstantExpression *>(expression)) {
        auto constant = constantExpression->getConstant();
        if (dynamic_cast<ConstantInt *>(constant)) {
            constantExpression->setExpType(std::make_unique<IntType>());
        }
        else if (dynamic_cast<ConstantLong *>(constant)) {
            constantExpression->setExpType(std::make_unique<LongType>());
        }
        else if (dynamic_cast<ConstantUInt *>(constant)) {
            constantExpression->setExpType(std::make_unique<UIntType>());
        }
        else if (dynamic_cast<ConstantULong *>(constant)) {
            constantExpression->setExpType(std::make_unique<ULongType>());
        }
        else {
            const auto &r = *constant;
            throw std::logic_error("Unsupported constant type in "
                                   "typeCheckExpression in TypeCheckingPass: " +
                                   std::string(typeid(r).name()));
        }
    }
    else if (auto variableExpression =
                 dynamic_cast<VariableExpression *>(expression)) {
        auto variableType =
            frontendSymbolTable[variableExpression->getIdentifier()]
                .first.get();
        // If the variable is not an arithmetic type, it is of type function.
        if (!isArithmeticType(variableType)) {
            std::stringstream msg;
            msg << "Function name used as variable in typeCheckExpression in "
                   "TypeCheckingPass: "
                << variableExpression->getIdentifier();
            throw std::logic_error(msg.str());
        }
        // Otherwise, set the expression type to the variable type.
        variableExpression->setExpType(cloneType(variableType));
    }
    else if (auto castExpression = dynamic_cast<CastExpression *>(expression)) {
        typeCheckExpression(castExpression->getExpression());
        castExpression->setExpType(cloneType(castExpression->getTargetType()));
    }
    else if (auto assignmentExpression =
                 dynamic_cast<AssignmentExpression *>(expression)) {
        auto left = assignmentExpression->getLeft();
        auto right = assignmentExpression->getRight();
        typeCheckExpression(left);
        typeCheckExpression(right);
        auto leftType = left->getExpType();
        assignmentExpression->setRight(convertTo(right, leftType));
        assignmentExpression->setExpType(cloneType(leftType));
    }
    else if (auto unaryExpression =
                 dynamic_cast<UnaryExpression *>(expression)) {
        typeCheckExpression(unaryExpression->getExpression());
        if (dynamic_cast<NotOperator *>(unaryExpression->getOperator())) {
            unaryExpression->setExpType(std::make_unique<IntType>());
        }
        else {
            unaryExpression->setExpType(
                cloneType(unaryExpression->getExpression()->getExpType()));
        }
    }
    else if (auto binaryExpression =
                 dynamic_cast<BinaryExpression *>(expression)) {
        typeCheckExpression(binaryExpression->getLeft());
        typeCheckExpression(binaryExpression->getRight());
        auto binaryOperator = binaryExpression->getOperator();
        if (dynamic_cast<AndOperator *>(binaryOperator) ||
            dynamic_cast<OrOperator *>(binaryOperator)) {
            // Logical operators should always return type `int`.
            binaryExpression->setExpType(std::make_unique<IntType>());
            return;
        }
        auto leftType = binaryExpression->getLeft()->getExpType();
        auto rightType = binaryExpression->getRight()->getExpType();
        auto commonType = getCommonType(leftType, rightType);
        binaryExpression->setLeft(
            convertTo(binaryExpression->getLeft(), commonType.get()));
        binaryExpression->setRight(
            convertTo(binaryExpression->getRight(), commonType.get()));
        if (dynamic_cast<AddOperator *>(binaryOperator) ||
            dynamic_cast<SubtractOperator *>(binaryOperator) ||
            dynamic_cast<MultiplyOperator *>(binaryOperator) ||
            dynamic_cast<DivideOperator *>(binaryOperator) ||
            dynamic_cast<RemainderOperator *>(binaryOperator)) {
            binaryExpression->setExpType(std::move(commonType));
        }
        else {
            binaryExpression->setExpType(std::make_unique<IntType>());
        }
    }
    else if (auto conditionalExpression =
                 dynamic_cast<ConditionalExpression *>(expression)) {
        typeCheckExpression(conditionalExpression->getCondition());
        typeCheckExpression(conditionalExpression->getThenExpression());
        typeCheckExpression(conditionalExpression->getElseExpression());
        auto thenType =
            conditionalExpression->getThenExpression()->getExpType();
        auto elseType =
            conditionalExpression->getElseExpression()->getExpType();
        // Get the common type of the then and else expressions/branches.
        auto commonType = getCommonType(thenType, elseType);
        // Convert the then and else expressions to the common type.
        conditionalExpression->setThenExpression(convertTo(
            conditionalExpression->getThenExpression(), commonType.get()));
        conditionalExpression->setElseExpression(convertTo(
            conditionalExpression->getElseExpression(), commonType.get()));
        // Set the conditional expression type to the common type.
        conditionalExpression->setExpType(std::move(commonType));
    }
}

void TypeCheckingPass::typeCheckStatement(
    Statement *statement, std::string enclosingFunctionIdentifier) {
    if (auto returnStatement = dynamic_cast<ReturnStatement *>(statement)) {
        // Look up the enclosing function's return type and convert the return
        // value to that type.
        // Use the enclosing function's name to look up the enclosing function's
        // return type.
        auto functionType =
            frontendSymbolTable[enclosingFunctionIdentifier].first.get();
        if (!functionType) {
            throw std::logic_error("Function not found in symbol table in "
                                   "typeCheckStatement in TypeCheckingPass: " +
                                   enclosingFunctionIdentifier);
        }
        if (isArithmeticType(functionType)) {
            throw std::logic_error("Function name used as variable in "
                                   "typeCheckStatement in TypeCheckingPass: " +
                                   enclosingFunctionIdentifier);
        }
        auto returnType = dynamic_cast<FunctionType *>(functionType);
        if (returnStatement->getExpression()) {
            typeCheckExpression(returnStatement->getExpression());
            returnStatement->setExpression(
                convertTo(returnStatement->getExpression(),
                          &returnType->getReturnType()));
        }
    }
    else if (auto expressionStatement =
                 dynamic_cast<ExpressionStatement *>(statement)) {
        typeCheckExpression(expressionStatement->getExpression());
    }
    else if (auto compoundStatement =
                 dynamic_cast<CompoundStatement *>(statement)) {
        typeCheckBlock(compoundStatement->getBlock(),
                       enclosingFunctionIdentifier);
    }
    else if (auto whileStatement = dynamic_cast<WhileStatement *>(statement)) {
        typeCheckExpression(whileStatement->getCondition());
        typeCheckStatement(whileStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto doWhileStatement =
                 dynamic_cast<DoWhileStatement *>(statement)) {
        typeCheckExpression(doWhileStatement->getCondition());
        typeCheckStatement(doWhileStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto forStatement = dynamic_cast<ForStatement *>(statement)) {
        if (forStatement->getForInit()) {
            typeCheckForInit(forStatement->getForInit());
        }
        if (forStatement->getOptCondition()) {
            typeCheckExpression(forStatement->getOptCondition());
        }
        if (forStatement->getOptPost()) {
            typeCheckExpression(forStatement->getOptPost());
        }
        typeCheckStatement(forStatement->getBody(),
                           enclosingFunctionIdentifier);
    }
    else if (auto ifStatement = dynamic_cast<IfStatement *>(statement)) {
        typeCheckExpression(ifStatement->getCondition());
        typeCheckStatement(ifStatement->getThenStatement(),
                           enclosingFunctionIdentifier);
        if (ifStatement->getElseOptStatement()) {
            typeCheckStatement(ifStatement->getElseOptStatement(),
                               enclosingFunctionIdentifier);
        }
    }
}

void TypeCheckingPass::typeCheckForInit(ForInit *forInit) {
    if (auto initExpr = dynamic_cast<InitExpr *>(forInit)) {
        if (initExpr->getExpression()) {
            typeCheckExpression(initExpr->getExpression());
        }
    }
    else if (auto initDecl = dynamic_cast<InitDecl *>(forInit)) {
        if (initDecl->getVariableDeclaration()->getOptStorageClass()) {
            throw std::logic_error("Storage class in for-init declaration in "
                                   "typeCheckForInit in TypeCheckingPass");
        }
        typeCheckLocalVariableDeclaration(initDecl->getVariableDeclaration());
    }
    else {
        throw std::logic_error("Unsupported for-init type for type-checking in "
                               "typeCheckForInit in TypeCheckingPass");
    }
}
/*
 * End: Functions for the type-checking pass.
 */

/*
 * Start: Functions for the loop-labeling pass.
 */
void LoopLabelingPass::labelLoops(Program &program) {
    for (auto &declaration : program.getDeclarations()) {
        if (auto *functionDeclaration =
                dynamic_cast<FunctionDeclaration *>(declaration.get())) {
            if (functionDeclaration->getOptBody()) {
                labelBlock(functionDeclaration->getOptBody(), "");
            }
        }
    }
}

std::string LoopLabelingPass::generateLoopLabel() {
    // Return a new label with the current counter value.
    return "loop" + std::to_string(loopLabelingCounter++);
}

void LoopLabelingPass::annotateStatement(Statement *statement,
                                         std::string_view label) {
    if (auto breakStatement = dynamic_cast<BreakStatement *>(statement)) {
        breakStatement->setLabel(label);
    }
    else if (auto continueStatement =
                 dynamic_cast<ContinueStatement *>(statement)) {
        continueStatement->setLabel(label);
    }
    else if (auto whileStatement = dynamic_cast<WhileStatement *>(statement)) {
        whileStatement->setLabel(label);
    }
    else if (auto doWhileStatement =
                 dynamic_cast<DoWhileStatement *>(statement)) {
        doWhileStatement->setLabel(label);
    }
    else if (auto forStatement = dynamic_cast<ForStatement *>(statement)) {
        forStatement->setLabel(label);
    }
}

void LoopLabelingPass::labelStatement(Statement *statement,
                                      std::string_view label) {
    if (auto breakStatement = dynamic_cast<BreakStatement *>(statement)) {
        if (label.empty()) {
            throw std::logic_error("Break statement outside of loop in "
                                   "labelStatement in LoopLabelingPass");
        }
        annotateStatement(breakStatement, label);
    }
    else if (auto continueStatement =
                 dynamic_cast<ContinueStatement *>(statement)) {
        if (label.empty()) {
            throw std::logic_error("Continue statement outside of loop in "
                                   "labelStatement in LoopLabelingPass");
        }
        annotateStatement(continueStatement, label);
    }
    else if (auto whileStatement = dynamic_cast<WhileStatement *>(statement)) {
        auto newLabel = generateLoopLabel();
        labelStatement(whileStatement->getBody(), newLabel);
        annotateStatement(whileStatement, newLabel);
    }
    else if (auto doWhileStatement =
                 dynamic_cast<DoWhileStatement *>(statement)) {
        auto newLabel = generateLoopLabel();
        labelStatement(doWhileStatement->getBody(), newLabel);
        annotateStatement(doWhileStatement, newLabel);
    }
    else if (auto forStatement = dynamic_cast<ForStatement *>(statement)) {
        auto newLabel = generateLoopLabel();
        labelStatement(forStatement->getBody(), newLabel);
        annotateStatement(forStatement, newLabel);
    }
    else if (auto ifStatement = dynamic_cast<IfStatement *>(statement)) {
        labelStatement(ifStatement->getThenStatement(), label);
        if (ifStatement->getElseOptStatement()) {
            labelStatement(ifStatement->getElseOptStatement(), label);
        }
    }
    else if (auto compoundStatement =
                 dynamic_cast<CompoundStatement *>(statement)) {
        labelBlock(compoundStatement->getBlock(), label);
    }
}

void LoopLabelingPass::labelBlock(Block *block, std::string_view label) {
    for (auto &blockItem : block->getBlockItems()) {
        if (dynamic_cast<DBlockItem *>(blockItem.get())) {
            continue;
        }
        if (auto sBlockItem = dynamic_cast<SBlockItem *>(blockItem.get())) {
            labelStatement(sBlockItem->getStatement(), label);
            continue;
        }
        throw std::logic_error("Unsupported block item type for loop labeling "
                               "in labelBlock in LoopLabelingPass: " +
                               std::string(typeid(blockItem).name()));
    }
}
/*
 * End: Functions for the loop-labeling pass.
 */
} // namespace AST
