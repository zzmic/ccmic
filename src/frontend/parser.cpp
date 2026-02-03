#include "parser.h"
#include "block.h"
#include "blockItem.h"
#include "constant.h"
#include "declaration.h"
#include "expression.h"
#include "forInit.h"
#include "lexer.h"
#include "program.h"
#include "statement.h"
#include "storageClass.h"
#include "type.h"
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace AST {
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

std::unique_ptr<Program> Parser::parse() {
    auto declarations =
        std::make_unique<std::vector<std::unique_ptr<Declaration>>>();
    while (current < tokens.size()) {
        auto declaration = parseDeclaration();
        declarations->emplace_back(std::move(declaration));
    }
    return std::make_unique<Program>(std::move(declarations));
}

bool Parser::matchToken(TokenType type) {
    // If the current index is less than the number of tokens and the current
    // token is of the expected type, return true. Otherwise, return false.
    if (current < tokens.size() && tokens[current].type == type) {
        return true;
    }
    return false;
}

Token Parser::consumeToken(TokenType type) {
    // If the current index is less than the number of tokens and the current
    // token is of the expected type, consume the token, increment the
    // current index, and return the token. Otherwise, throw an error.
    if (matchToken(type)) {
        return tokens[current++];
    }
    std::stringstream msg;
    msg << "Expect token of type " << tokenTypeToString(type) << " but found "
        << tokens[current].value;
    msg << " of type " << tokenTypeToString(tokens[current].type)
        << " in consumeToken in Parser";
    throw std::invalid_argument(msg.str());
}

void Parser::expectToken(TokenType type) {
    // If the current token is not of the expected type, throw an error.
    // Otherwise, consume the token.
    if (!matchToken(type)) {
        std::stringstream msg;
        msg << "Expect token of type " << tokenTypeToString(type)
            << " but found " << tokens[current].value;
        msg << " of type " << tokenTypeToString(tokens[current].type)
            << " in expectToken in Parser";
        throw std::invalid_argument(msg.str());
    }
    consumeToken(type);
}

std::unique_ptr<BlockItem> Parser::parseBlockItem() {
    if (matchToken(TokenType::intKeyword) ||
        matchToken(TokenType::longKeyword) ||
        matchToken(TokenType::signedKeyword) ||
        matchToken(TokenType::unsignedKeyword) ||
        matchToken(TokenType::staticKeyword) ||
        matchToken(TokenType::externKeyword)) {
        // Peek ahead to check if this is a function declaration.
        size_t nextIndex = current + 1;
        if (nextIndex < tokens.size() &&
            tokens[nextIndex].type == TokenType::Identifier) {
            size_t afterIdentifierIndex = nextIndex + 1;
            if (afterIdentifierIndex < tokens.size() &&
                tokens[afterIdentifierIndex].type ==
                    TokenType::OpenParenthesis) {
                // If the sequence matches `int <identifier> (<params>)`, it's a
                // function declaration.
                auto functionDeclaration = parseDeclaration();
                return std::make_unique<DBlockItem>(
                    std::move(functionDeclaration));
            }
        }
        // Otherwise, treat it as a variable declaration.
        auto declaration = parseDeclaration();
        return std::make_unique<DBlockItem>(std::move(declaration));
    }
    else {
        auto statement = parseStatement();
        return std::make_unique<SBlockItem>(std::move(statement));
    }
}

std::unique_ptr<Block> Parser::parseBlock() {
    expectToken(TokenType::OpenBrace);
    auto blockItems =
        std::make_unique<std::vector<std::unique_ptr<BlockItem>>>();
    while (!matchToken(TokenType::CloseBrace)) {
        auto nextBlockItem = parseBlockItem();
        blockItems->emplace_back(std::move(nextBlockItem));
    }
    expectToken(TokenType::CloseBrace);
    return std::make_unique<Block>(std::move(blockItems));
}

std::unique_ptr<Declaration> Parser::parseDeclaration() {
    // Parse the specifier list.
    std::vector<std::string> specifierList;
    while (matchToken(TokenType::intKeyword) ||
           matchToken(TokenType::longKeyword) ||
           matchToken(TokenType::signedKeyword) ||
           matchToken(TokenType::unsignedKeyword) ||
           matchToken(TokenType::staticKeyword) ||
           matchToken(TokenType::externKeyword)) {
        if (matchToken(TokenType::intKeyword)) {
            specifierList.emplace_back("int");
            consumeToken(TokenType::intKeyword);
        }
        else if (matchToken(TokenType::longKeyword)) {
            specifierList.emplace_back("long");
            consumeToken(TokenType::longKeyword);
        }
        else if (matchToken(TokenType::signedKeyword)) {
            specifierList.emplace_back("signed");
            consumeToken(TokenType::signedKeyword);
        }
        else if (matchToken(TokenType::unsignedKeyword)) {
            specifierList.emplace_back("unsigned");
            consumeToken(TokenType::unsignedKeyword);
        }
        else if (matchToken(TokenType::staticKeyword)) {
            specifierList.emplace_back("static");
            consumeToken(TokenType::staticKeyword);
        }
        else if (matchToken(TokenType::externKeyword)) {
            specifierList.emplace_back("extern");
            consumeToken(TokenType::externKeyword);
        }
    }
    // Parse the type and storage class.
    auto typesAndStorageClass = parseTypeAndStorageClass(specifierList);
    auto type = std::move(typesAndStorageClass.first);
    auto storageClass = std::move(typesAndStorageClass.second);
    // Consume an identifier token (common to both variable and function
    // declarations).
    auto identifierToken = consumeToken(TokenType::Identifier);
    if (matchToken(TokenType::OpenParenthesis)) {
        // Parse a function declaration.
        expectToken(TokenType::OpenParenthesis);
        auto parameters = std::make_unique<std::vector<std::string>>();
        auto parameterTypes =
            std::make_unique<std::vector<std::unique_ptr<Type>>>();

        if (matchToken(TokenType::voidKeyword)) {
            consumeToken(TokenType::voidKeyword);
        }
        else if (matchToken(TokenType::intKeyword) ||
                 matchToken(TokenType::longKeyword) ||
                 matchToken(TokenType::signedKeyword) ||
                 matchToken(TokenType::unsignedKeyword)) {
            auto specifiers = parseTypeSpecifiersInParameters();
            auto parameterNameToken1 = consumeToken(TokenType::Identifier);
            parameters->emplace_back(parameterNameToken1.value);
            parameterTypes->emplace_back(parseType(specifiers));
            // Parse additional parameters if they exist.
            while (matchToken(TokenType::Comma)) {
                consumeToken(TokenType::Comma);
                auto paramSpecifiers = parseTypeSpecifiersInParameters();
                auto parameterNameToken2 = consumeToken(TokenType::Identifier);
                parameters->emplace_back(parameterNameToken2.value);
                parameterTypes->emplace_back(parseType(paramSpecifiers));
            }
        }
        expectToken(TokenType::CloseParenthesis);

        // Create a proper `FunctionType` with parameter types and return type.
        auto functionType = std::make_unique<FunctionType>(
            std::move(parameterTypes), std::move(type));

        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
            if (storageClass) {
                return std::make_unique<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionType), std::move(storageClass));
            }
            else {
                return std::make_unique<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionType));
            }
        }
        else {
            auto functionBody = parseBlock();
            if (storageClass) {
                return std::make_unique<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionBody), std::move(functionType),
                    std::move(storageClass));
            }
            else {
                return std::make_unique<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionBody), std::move(functionType));
            }
        }
    }
    else {
        // Parse a variable declaration.
        if (matchToken(TokenType::Assign)) {
            consumeToken(TokenType::Assign);
            auto expr = parseExpression(0);
            expectToken(TokenType::Semicolon);
            if (storageClass) {
                return std::make_unique<VariableDeclaration>(
                    identifierToken.value, std::move(expr), std::move(type),
                    std::move(storageClass));
            }
            else {
                return std::make_unique<VariableDeclaration>(
                    identifierToken.value, std::move(expr), std::move(type));
            }
        }
        else {
            expectToken(TokenType::Semicolon);
            if (storageClass) {
                return std::make_unique<VariableDeclaration>(
                    identifierToken.value, std::move(type),
                    std::move(storageClass));
            }
            else {
                return std::make_unique<VariableDeclaration>(
                    identifierToken.value, std::move(type));
            }
        }
    }
}

std::vector<std::string> Parser::parseTypeSpecifiersInParameters() {
    std::vector<std::string> specifiers;
    while (matchToken(TokenType::intKeyword) ||
           matchToken(TokenType::longKeyword) ||
           matchToken(TokenType::signedKeyword) ||
           matchToken(TokenType::unsignedKeyword)) {
        if (matchToken(TokenType::intKeyword)) {
            expectToken(TokenType::intKeyword);
            specifiers.emplace_back("int");
        }
        else if (matchToken(TokenType::longKeyword)) {
            expectToken(TokenType::longKeyword);
            specifiers.emplace_back("long");
        }
        else if (matchToken(TokenType::signedKeyword)) {
            expectToken(TokenType::signedKeyword);
            specifiers.emplace_back("signed");
        }
        else if (matchToken(TokenType::unsignedKeyword)) {
            expectToken(TokenType::unsignedKeyword);
            specifiers.emplace_back("unsigned");
        }
    }
    if (specifiers.empty()) {
        throw std::invalid_argument(
            "Missing type specifier in parameter in "
            "parseTypeSpecifiersInParameters in Parser");
    }
    return specifiers;
}

std::unique_ptr<ForInit> Parser::parseForInit() {
    if (matchToken(TokenType::intKeyword) ||
        matchToken(TokenType::longKeyword) ||
        matchToken(TokenType::signedKeyword) ||
        matchToken(TokenType::unsignedKeyword) ||
        matchToken(TokenType::staticKeyword) ||
        matchToken(TokenType::externKeyword)) {
        auto declaration = parseDeclaration();
        if (dynamic_cast<VariableDeclaration *>(declaration.get())) {
            // Release as Declaration and re-wrap as VariableDeclaration.
            auto *varDecl =
                static_cast<VariableDeclaration *>(declaration.release());
            return std::make_unique<InitDecl>(
                std::unique_ptr<VariableDeclaration>(varDecl));
        }
        else {
            throw std::invalid_argument(
                "Function declarations aren't permitted in for-loop headers in "
                "parseForInit in Parser");
        }
    }
    else {
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
            return std::make_unique<InitExpr>(nullptr);
        }
        else {
            std::unique_ptr<Expression> expr = parseExpression(0);
            expectToken(TokenType::Semicolon);
            return std::make_unique<InitExpr>(std::move(expr));
        }
    }
}

std::unique_ptr<Statement> Parser::parseStatement() {
    // Parse a return statement.
    if (matchToken(TokenType::returnKeyword)) {
        consumeToken(TokenType::returnKeyword);
        auto expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_unique<ReturnStatement>(std::move(expr));
    }
    // Parse a null statement.
    else if (matchToken(TokenType::Semicolon)) {
        consumeToken(TokenType::Semicolon);
        return std::make_unique<NullStatement>();
    }
    // Parse an if-statement.
    else if (matchToken(TokenType::ifKeyword)) {
        consumeToken(TokenType::ifKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        auto thenStatement = parseStatement();
        if (matchToken(TokenType::elseKeyword)) {
            consumeToken(TokenType::elseKeyword);
            auto elseStatement = parseStatement();
            return std::make_unique<IfStatement>(std::move(condition),
                                                 std::move(thenStatement),
                                                 std::move(elseStatement));
        }
        return std::make_unique<IfStatement>(std::move(condition),
                                             std::move(thenStatement));
    }
    // Parse a compound statement.
    else if (matchToken(TokenType::OpenBrace)) {
        auto block = parseBlock();
        return std::make_unique<CompoundStatement>(std::move(block));
    }
    // Parse a break statement.
    else if (matchToken(TokenType::breakKeyword)) {
        consumeToken(TokenType::breakKeyword);
        expectToken(TokenType::Semicolon);
        return std::make_unique<BreakStatement>();
    }
    // Parse a continue statement.
    else if (matchToken(TokenType::continueKeyword)) {
        consumeToken(TokenType::continueKeyword);
        expectToken(TokenType::Semicolon);
        return std::make_unique<ContinueStatement>();
    }
    // Parse a while-statement.
    else if (matchToken(TokenType::whileKeyword)) {
        consumeToken(TokenType::whileKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        auto body = parseStatement();
        return std::make_unique<WhileStatement>(std::move(condition),
                                                std::move(body));
    }
    // Parse a do-while-statement.
    else if (matchToken(TokenType::doKeyword)) {
        consumeToken(TokenType::doKeyword);
        auto body = parseStatement();
        expectToken(TokenType::whileKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        expectToken(TokenType::Semicolon);
        return std::make_unique<DoWhileStatement>(std::move(condition),
                                                  std::move(body));
    }
    // Parse a for-statement.
    else if (matchToken(TokenType::forKeyword)) {
        consumeToken(TokenType::forKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto init = parseForInit();
        std::unique_ptr<Expression> optCondition = nullptr;
        std::unique_ptr<Expression> optPost = nullptr;
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
        }
        else {
            optCondition = parseExpression(0);
            expectToken(TokenType::Semicolon);
        }
        if (matchToken(TokenType::CloseParenthesis)) {
            consumeToken(TokenType::CloseParenthesis);
        }
        else {
            optPost = parseExpression(0);
            expectToken(TokenType::CloseParenthesis);
        }
        auto body = parseStatement();
        return std::make_unique<ForStatement>(
            std::move(init), std::move(optCondition), std::move(optPost),
            std::move(body));
    }
    // Parse an expression statement.
    else {
        auto expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }
    std::stringstream msg;
    msg << "Malformed statement: unexpected token: " << tokens[current].value;
    msg << " of type " << tokenTypeToString(tokens[current].type)
        << " in parseStatement in Parser";
    throw std::invalid_argument(msg.str());
}

std::unique_ptr<Expression> Parser::parseFactor() {
    if (matchToken(TokenType::IntConstant) ||
        matchToken(TokenType::LongConstant) ||
        matchToken(TokenType::UnsignedIntegerConstant) ||
        matchToken(TokenType::UnsignedLongIntegerConstant)) {
        return std::make_unique<ConstantExpression>(parseConstant());
    }
    else if (matchToken(TokenType::Identifier)) {
        auto identifierToken = consumeToken(TokenType::Identifier);
        // Parse a function-call expression.
        if (matchToken(TokenType::OpenParenthesis)) {
            consumeToken(TokenType::OpenParenthesis);
            auto arguments =
                std::make_unique<std::vector<std::unique_ptr<Expression>>>();
            if (!matchToken(TokenType::CloseParenthesis)) {
                // Parse additional arguments if they exist.
                do {
                    auto argument = parseExpression(0);
                    arguments->emplace_back(std::move(argument));
                    if (matchToken(TokenType::Comma)) {
                        consumeToken(TokenType::Comma);
                    }
                    else {
                        break;
                    }
                } while (true);
            }
            expectToken(TokenType::CloseParenthesis);
            return std::make_unique<FunctionCallExpression>(
                identifierToken.value, std::move(arguments));
        }
        // Parse a variable expression.
        else {
            return std::make_unique<VariableExpression>(identifierToken.value);
        }
    }
    else if (matchToken(TokenType::OpenParenthesis) &&
             current + 1 < tokens.size() &&
             (tokens[current + 1].type == TokenType::intKeyword ||
              tokens[current + 1].type == TokenType::longKeyword ||
              tokens[current + 1].type == TokenType::signedKeyword ||
              tokens[current + 1].type == TokenType::unsignedKeyword)) {
        consumeToken(TokenType::OpenParenthesis);
        std::vector<std::string> specifierList;
        while (matchToken(TokenType::intKeyword) ||
               matchToken(TokenType::longKeyword) ||
               matchToken(TokenType::signedKeyword) ||
               matchToken(TokenType::unsignedKeyword)) {
            if (matchToken(TokenType::intKeyword)) {
                specifierList.emplace_back("int");
                consumeToken(TokenType::intKeyword);
            }
            else if (matchToken(TokenType::longKeyword)) {
                specifierList.emplace_back("long");
                consumeToken(TokenType::longKeyword);
            }
            else if (matchToken(TokenType::signedKeyword)) {
                specifierList.emplace_back("signed");
                consumeToken(TokenType::signedKeyword);
            }
            else if (matchToken(TokenType::unsignedKeyword)) {
                specifierList.emplace_back("unsigned");
                consumeToken(TokenType::unsignedKeyword);
            }
        }
        consumeToken(TokenType::CloseParenthesis);
        auto targetType = parseType(specifierList);
        auto innerExpr = parseFactor();
        return std::make_unique<CastExpression>(std::move(targetType),
                                                std::move(innerExpr));
    }
    else if (matchToken(TokenType::Tilde)) {
        auto tildeToken = consumeToken(TokenType::Tilde);
        auto innerExpr = parseFactor();
        return std::make_unique<UnaryExpression>(tildeToken.value,
                                                 std::move(innerExpr));
    }
    else if (matchToken(TokenType::Minus)) {
        auto minusToken = consumeToken(TokenType::Minus);
        auto innerExpr = parseFactor();
        return std::make_unique<UnaryExpression>(minusToken.value,
                                                 std::move(innerExpr));
    }
    else if (matchToken(TokenType::LogicalNot)) {
        auto notToken = consumeToken(TokenType::LogicalNot);
        auto innerExpr = parseFactor();
        return std::make_unique<UnaryExpression>(notToken.value,
                                                 std::move(innerExpr));
    }
    else if (matchToken(TokenType::OpenParenthesis)) {
        consumeToken(TokenType::OpenParenthesis);
        auto innerExpr = parseExpression(0);
        if (matchToken(TokenType::CloseParenthesis)) {
            consumeToken(TokenType::CloseParenthesis);
            return innerExpr;
        }
        else {
            throw std::invalid_argument(
                "Malformed factor: missing closing parenthesis.");
        }
    }
    else {
        std::stringstream msg;
        msg << "Malformed factor: unexpected token: " << tokens[current].value;
        msg << " of type " << tokenTypeToString(tokens[current].type);
        msg << " in parseFactor in Parser";
        throw std::invalid_argument(msg.str());
    }
}

std::unique_ptr<Constant> Parser::parseConstant() {
    // 2^63 - 1 = 9223372036854775807LL.
    constexpr auto MAX_LONG = 9223372036854775807LL;
    // 2^31 - 1 = 2147483647LL.
    constexpr auto MAX_INT = 2147483647LL;
    // 2^64 - 1 = 18446744073709551615ULL.
    constexpr unsigned long long MAX_ULONG = 18446744073709551615ULL;
    // 2^32 - 1 = 4294967295ULL.
    constexpr unsigned long long MAX_UINT = 4294967295ULL;

    if (tokens[current].type == TokenType::UnsignedIntegerConstant ||
        tokens[current].type == TokenType::UnsignedLongIntegerConstant) {
        auto constantValue = std::stoull(tokens[current].value);
        if (constantValue > MAX_ULONG) {
            throw std::invalid_argument(
                "Constant is too large to represent as an unsigned long in "
                "parseConstant in Parser");
        }
        if (tokens[current].type == TokenType::UnsignedIntegerConstant) {
            if (constantValue <= MAX_UINT) {
                consumeToken(TokenType::UnsignedIntegerConstant);
                return std::make_unique<ConstantUInt>(
                    static_cast<unsigned int>(constantValue));
            }
            consumeToken(TokenType::UnsignedIntegerConstant);
            return std::make_unique<ConstantULong>(
                static_cast<unsigned long>(constantValue));
        }
        consumeToken(TokenType::UnsignedLongIntegerConstant);
        return std::make_unique<ConstantULong>(
            static_cast<unsigned long>(constantValue));
    }

    auto constantValue = std::stoll(tokens[current].value);
    if (constantValue > MAX_LONG) {
        throw std::invalid_argument("Constant is too large to represent as an "
                                    "int or long in parseConstant in Parser");
    }
    if (tokens[current].type == TokenType::IntConstant) {
        if (constantValue <= MAX_INT) {
            consumeToken(TokenType::IntConstant);
            return std::make_unique<ConstantInt>(
                static_cast<int>(constantValue));
        }
        consumeToken(TokenType::IntConstant);
        return std::make_unique<ConstantLong>(constantValue);
    }
    consumeToken(TokenType::LongConstant);
    return std::make_unique<ConstantLong>(constantValue);
}

std::unique_ptr<Expression> Parser::parseExpression(int minPrecedence) {
    // Parse the left operand of the expression.
    std::unique_ptr<Expression> left = parseFactor();
    // While the next token is a binary operator and has a precedence greater
    // than or equal to the minimum precedence, ...
    while (
        (matchToken(TokenType::Plus) || matchToken(TokenType::Minus) ||
         matchToken(TokenType::Multiply) || matchToken(TokenType::Divide) ||
         matchToken(TokenType::Modulo) || matchToken(TokenType::LogicalAnd) ||
         matchToken(TokenType::LogicalOr) || matchToken(TokenType::Equal) ||
         matchToken(TokenType::NotEqual) || matchToken(TokenType::LessThan) ||
         matchToken(TokenType::LessThanOrEqual) ||
         matchToken(TokenType::GreaterThan) ||
         matchToken(TokenType::GreaterThanOrEqual) ||
         matchToken(TokenType::Assign) ||
         matchToken(TokenType::QuestionMark)) &&
        getPrecedence(tokens[current]) >= minPrecedence) {
        // If the next token is an assignment operator, ...
        if (matchToken(TokenType::Assign)) {
            auto assignToken = consumeToken(TokenType::Assign);
            auto right = parseExpression(getPrecedence(assignToken));
            left = std::make_unique<AssignmentExpression>(std::move(left),
                                                          std::move(right));
        }
        else if (matchToken(TokenType::QuestionMark)) {
            auto questionMarkToken = consumeToken(TokenType::QuestionMark);
            auto middle = parseConditionalMiddle();
            auto right = parseExpression(getPrecedence(questionMarkToken));
            left = std::make_unique<ConditionalExpression>(
                std::move(left), std::move(middle), std::move(right));
        }
        // Otherwise, the next token is (should be) a binary operator.
        else {
            auto binOpToken = consumeToken(tokens[current].type);
            if (!(matchToken(TokenType::IntConstant) ||
                  matchToken(TokenType::LongConstant) ||
                  matchToken(TokenType::UnsignedIntegerConstant) ||
                  matchToken(TokenType::UnsignedLongIntegerConstant) ||
                  matchToken(TokenType::Tilde) ||
                  matchToken(TokenType::Minus) ||
                  matchToken(TokenType::LogicalNot) ||
                  matchToken(TokenType::OpenParenthesis) ||
                  matchToken(TokenType::Identifier))) {
                std::stringstream msg;
                msg << "Malformed expression: binary operator "
                    << binOpToken.value
                    << " is not followed by a valid operand.";
                throw std::invalid_argument(msg.str());
            }
            std::unique_ptr<Expression> right =
                parseExpression(getPrecedence(binOpToken) + 1);
            left = std::make_unique<BinaryExpression>(
                std::move(left), binOpToken.value, std::move(right));
        }
    }
    return left;
}

std::unique_ptr<Expression> Parser::parseConditionalMiddle() {
    // Note: The question mark token has already been consumed in the caller
    // (some `parseExpression` function call). Parse the middle expression.
    auto middle = parseExpression(0);
    // Consume the colon token.
    consumeToken(TokenType::Colon);
    return middle;
}

std::pair<std::unique_ptr<Type>, std::unique_ptr<StorageClass>>
Parser::parseTypeAndStorageClass(
    const std::vector<std::string> &specifierList) {
    std::vector<std::string> types;
    std::vector<std::string> storageClasses;
    for (const auto &specifier : specifierList) {
        if (specifier == "int" || specifier == "long" ||
            specifier == "signed" || specifier == "unsigned") {
            types.emplace_back(specifier);
        }
        else {
            storageClasses.emplace_back(specifier);
        }
    }
    auto type = parseType(types);
    std::unique_ptr<StorageClass> storageClass;
    if (storageClasses.size() > 1) {
        throw std::invalid_argument("Invalid storage class (specifier) in "
                                    "parseTypeAndStorageClass in Parser");
    }
    if (storageClasses.size() == 1) {
        storageClass = parseStorageClass(storageClasses[0]);
    }
    else {
        storageClass = nullptr;
    }
    return std::make_pair(std::move(type), std::move(storageClass));
}

std::unique_ptr<Type>
Parser::parseType(const std::vector<std::string> &specifierList) {
    const std::unordered_set<std::string> specifierSet(specifierList.begin(),
                                                       specifierList.end());
    if (specifierSet.empty()) {
        throw std::invalid_argument(
            "Invalid type specifier (empty) in parseType in Parser");
    }
    else if (specifierSet.size() != specifierList.size()) {
        throw std::invalid_argument("Invalid type specifier (duplicate "
                                    "specifiers) in parseType in Parser");
    }
    else if (specifierSet.contains("signed") &&
             specifierSet.contains("unsigned")) {
        throw std::invalid_argument("Invalid type specifier (both signed and "
                                    "unsigned) in parseType in Parser");
    }

    if (specifierSet.contains("unsigned") && specifierSet.contains("long")) {
        return std::make_unique<ULongType>();
    }
    else if (specifierSet.contains("unsigned")) {
        return std::make_unique<UIntType>();
    }
    else if (specifierSet.contains("long")) {
        return std::make_unique<LongType>();
    }
    else if (specifierSet.contains("int") || specifierSet.contains("signed")) {
        return std::make_unique<IntType>();
    }
    else {
        std::stringstream msg;
        msg << "Invalid type specifier: ";
        for (const auto &specifier : specifierList) {
            msg << specifier << " ";
        }
        throw std::invalid_argument(msg.str());
    }
}

std::unique_ptr<StorageClass>
Parser::parseStorageClass(const std::string &specifier) {
    if (specifier == "static") {
        return std::make_unique<StaticStorageClass>();
    }
    else if (specifier == "extern") {
        return std::make_unique<ExternStorageClass>();
    }
    else {
        std::stringstream msg;
        msg << "Invalid storage class (specifier) in parseStorageClass in "
               "Parser: "
            << specifier;
        throw std::invalid_argument(msg.str());
    }
}

int Parser::getPrecedence(const Token &token) {
    if (precedenceMap.find(token.type) != precedenceMap.end()) {
        return precedenceMap[token.type];
    }
    return -1;
}
} // namespace AST
