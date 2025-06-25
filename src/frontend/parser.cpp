#include "parser.h"
#include <cmath>
#include <sstream>

namespace AST {
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

std::shared_ptr<Program> Parser::parse() {
    auto declarations =
        std::make_shared<std::vector<std::shared_ptr<Declaration>>>();
    while (current < tokens.size()) {
        auto declaration = parseDeclaration();
        declarations->emplace_back(std::move(declaration));
    }
    return std::make_shared<Program>(std::move(declarations));
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
    msg << " of type " << tokenTypeToString(tokens[current].type);
    throw std::runtime_error(msg.str());
}

void Parser::expectToken(TokenType type) {
    // If the current token is not of the expected type, throw an error.
    // Otherwise, consume the token.
    if (!matchToken(type)) {
        std::stringstream msg;
        msg << "Expect token of type " << tokenTypeToString(type)
            << " but found " << tokens[current].value;
        msg << " of type " << tokenTypeToString(tokens[current].type);
        throw std::runtime_error(msg.str());
    }
    consumeToken(type);
}

std::shared_ptr<BlockItem> Parser::parseBlockItem() {
    if (matchToken(TokenType::intKeyword) ||
        matchToken(TokenType::longKeyword) ||
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
                return std::make_shared<DBlockItem>(
                    std::move(functionDeclaration));
            }
        }
        // Otherwise, treat it as a variable declaration.
        auto declaration = parseDeclaration();
        return std::make_shared<DBlockItem>(std::move(declaration));
    }
    else {
        auto statement = parseStatement();
        return std::make_shared<SBlockItem>(std::move(statement));
    }
}

std::shared_ptr<Block> Parser::parseBlock() {
    expectToken(TokenType::OpenBrace);
    auto blockItems =
        std::make_shared<std::vector<std::shared_ptr<BlockItem>>>();
    while (!matchToken(TokenType::CloseBrace)) {
        auto nextBlockItem = parseBlockItem();
        blockItems->emplace_back(std::move(nextBlockItem));
    }
    expectToken(TokenType::CloseBrace);
    return std::make_shared<Block>(std::move(blockItems));
}

std::shared_ptr<Declaration> Parser::parseDeclaration() {
    // Parse the specifier list.
    std::vector<std::string> specifierList;
    while (matchToken(TokenType::intKeyword) ||
           matchToken(TokenType::longKeyword) ||
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
    auto type = typesAndStorageClass.first;
    auto storageClass = typesAndStorageClass.second;
    // Consume an identifier token (common to both variable and function
    // declarations).
    auto identifierToken = consumeToken(TokenType::Identifier);
    if (matchToken(TokenType::OpenParenthesis)) {
        // Parse a function declaration.
        expectToken(TokenType::OpenParenthesis);
        auto parameters = std::make_shared<std::vector<std::string>>();
        auto parameterTypes =
            std::make_shared<std::vector<std::shared_ptr<Type>>>();

        if (matchToken(TokenType::voidKeyword)) {
            consumeToken(TokenType::voidKeyword);
        }
        else if (matchToken(TokenType::intKeyword)) {
            parseTypeSpecifiersInParameters(parameters);
            auto parameterNameToken = consumeToken(TokenType::Identifier);
            parameters->emplace_back(parameterNameToken.value);
            parameterTypes->emplace_back(std::make_shared<IntType>());
            // Parse additional parameters if they exist.
            while (matchToken(TokenType::Comma)) {
                consumeToken(TokenType::Comma);
                parseTypeSpecifiersInParameters(parameters);
                auto parameterNameToken = consumeToken(TokenType::Identifier);
                parameters->emplace_back(parameterNameToken.value);
                parameterTypes->emplace_back(std::make_shared<IntType>());
            }
        }
        else if (matchToken(TokenType::longKeyword)) {
            parseTypeSpecifiersInParameters(parameters);
            auto parameterNameToken = consumeToken(TokenType::Identifier);
            parameters->emplace_back(parameterNameToken.value);
            parameterTypes->emplace_back(std::make_shared<LongType>());
            // Parse additional parameters if they exist.
            while (matchToken(TokenType::Comma)) {
                consumeToken(TokenType::Comma);
                parseTypeSpecifiersInParameters(parameters);
                auto parameterNameToken = consumeToken(TokenType::Identifier);
                parameters->emplace_back(parameterNameToken.value);
                parameterTypes->emplace_back(std::make_shared<LongType>());
            }
        }
        expectToken(TokenType::CloseParenthesis);

        // Create a proper `FunctionType` with parameter types and return type.
        auto functionType =
            std::make_shared<FunctionType>(parameterTypes, type);

        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
            if (storageClass) {
                return std::make_shared<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionType),
                    std::make_optional(std::move(storageClass)));
            }
            else {
                return std::make_shared<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::move(functionType));
            }
        }
        else {
            auto functionBody = parseBlock();
            if (storageClass) {
                return std::make_shared<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::make_optional(std::move(functionBody)),
                    std::move(functionType),
                    std::make_optional(std::move(storageClass)));
            }
            else {
                return std::make_shared<FunctionDeclaration>(
                    identifierToken.value, std::move(parameters),
                    std::make_optional(std::move(functionBody)),
                    std::move(functionType));
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
                return std::make_shared<VariableDeclaration>(
                    identifierToken.value, std::move(expr), std::move(type),
                    std::move(storageClass));
            }
            else {
                return std::make_shared<VariableDeclaration>(
                    identifierToken.value, std::move(expr), std::move(type));
            }
        }
        else {
            expectToken(TokenType::Semicolon);
            if (storageClass) {
                return std::make_shared<VariableDeclaration>(
                    identifierToken.value, std::move(type),
                    std::move(storageClass));
            }
            else {
                return std::make_shared<VariableDeclaration>(
                    identifierToken.value, std::move(type));
            }
        }
    }
}

void Parser::parseTypeSpecifiersInParameters(
    const std::shared_ptr<std::vector<std::string>> &parameters) {
    while (matchToken(TokenType::intKeyword) ||
           matchToken(TokenType::longKeyword)) {
        if (matchToken(TokenType::intKeyword)) {
            expectToken(TokenType::intKeyword);
            parameters->emplace_back("int");
        }
        else if (matchToken(TokenType::longKeyword)) {
            expectToken(TokenType::longKeyword);
            parameters->emplace_back("long");
        }
    }
}

std::shared_ptr<ForInit> Parser::parseForInit() {
    if (matchToken(TokenType::intKeyword) ||
        matchToken(TokenType::longKeyword) ||
        matchToken(TokenType::staticKeyword) ||
        matchToken(TokenType::externKeyword)) {
        auto declaration = parseDeclaration();
        if (std::dynamic_pointer_cast<VariableDeclaration>(declaration)) {
            return std::make_shared<InitDecl>(
                std::static_pointer_cast<VariableDeclaration>(declaration));
        }
        else {
            throw std::runtime_error(
                "Function declarations aren't permitted in for-loop headers");
        }
        return std::make_shared<InitDecl>(
            std::static_pointer_cast<VariableDeclaration>(declaration));
    }
    else {
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
            return std::make_shared<InitExpr>();
        }
        else {
            std::shared_ptr<Expression> expr = parseExpression(0);
            expectToken(TokenType::Semicolon);
            return std::make_shared<InitExpr>(std::move(expr));
        }
    }
}

std::shared_ptr<Statement> Parser::parseStatement() {
    // Parse a return statement.
    if (matchToken(TokenType::returnKeyword)) {
        consumeToken(TokenType::returnKeyword);
        auto expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<ReturnStatement>(std::move(expr));
    }
    // Parse a null statement.
    else if (matchToken(TokenType::Semicolon)) {
        consumeToken(TokenType::Semicolon);
        return std::make_shared<NullStatement>();
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
            return std::make_shared<IfStatement>(std::move(condition),
                                                 std::move(thenStatement),
                                                 std::move(elseStatement));
        }
        return std::make_shared<IfStatement>(std::move(condition),
                                             std::move(thenStatement));
    }
    // Parse a compound statement.
    else if (matchToken(TokenType::OpenBrace)) {
        auto block = parseBlock();
        return std::make_shared<CompoundStatement>(std::move(block));
    }
    // Parse a break statement.
    else if (matchToken(TokenType::breakKeyword)) {
        consumeToken(TokenType::breakKeyword);
        expectToken(TokenType::Semicolon);
        return std::make_shared<BreakStatement>();
    }
    // Parse a continue statement.
    else if (matchToken(TokenType::continueKeyword)) {
        consumeToken(TokenType::continueKeyword);
        expectToken(TokenType::Semicolon);
        return std::make_shared<ContinueStatement>();
    }
    // Parse a while-statement.
    else if (matchToken(TokenType::whileKeyword)) {
        consumeToken(TokenType::whileKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        auto body = parseStatement();
        return std::make_shared<WhileStatement>(std::move(condition),
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
        return std::make_shared<DoWhileStatement>(std::move(condition),
                                                  std::move(body));
    }
    // Parse a for-statement.
    else if (matchToken(TokenType::forKeyword)) {
        consumeToken(TokenType::forKeyword);
        expectToken(TokenType::OpenParenthesis);
        auto init = parseForInit();
        std::optional<std::shared_ptr<Expression>> optCondition;
        std::optional<std::shared_ptr<Expression>> optPost;
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
        }
        else {
            auto condition = parseExpression(0);
            optCondition = std::make_optional(std::move(condition));
            expectToken(TokenType::Semicolon);
        }
        if (matchToken(TokenType::CloseParenthesis)) {
            consumeToken(TokenType::CloseParenthesis);
        }
        else {
            auto post = parseExpression(0);
            optPost = std::make_optional(std::move(post));
            expectToken(TokenType::CloseParenthesis);
        }
        auto body = parseStatement();
        return std::make_shared<ForStatement>(
            std::move(init), std::move(optCondition), std::move(optPost),
            std::move(body));
    }
    // Parse an expression statement.
    else {
        auto expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<ExpressionStatement>(std::move(expr));
    }
    std::stringstream msg;
    msg << "Malformed statement: unexpected token: " << tokens[current].value;
    msg << " of type " << tokenTypeToString(tokens[current].type);
    throw std::runtime_error(msg.str());
}

std::shared_ptr<Expression> Parser::parseFactor() {
    if (matchToken(TokenType::IntConstant) ||
        matchToken(TokenType::LongConstant)) {
        return std::make_shared<ConstantExpression>(parseConstant());
    }
    else if (matchToken(TokenType::Identifier)) {
        auto identifierToken = consumeToken(TokenType::Identifier);
        // Parse a function-call expression.
        if (matchToken(TokenType::OpenParenthesis)) {
            consumeToken(TokenType::OpenParenthesis);
            auto arguments =
                std::make_shared<std::vector<std::shared_ptr<Expression>>>();
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
            return std::make_shared<FunctionCallExpression>(
                identifierToken.value, std::move(arguments));
        }
        // Parse a variable expression.
        else {
            return std::make_shared<VariableExpression>(identifierToken.value);
        }
    }
    else if (matchToken(TokenType::OpenParenthesis) &&
             (tokens[current + 1].type == TokenType::intKeyword ||
              tokens[current + 1].type == TokenType::longKeyword)) {
        consumeToken(TokenType::OpenParenthesis);
        std::vector<std::string> specifierList;
        while (matchToken(TokenType::intKeyword) ||
               matchToken(TokenType::longKeyword)) {
            if (matchToken(TokenType::intKeyword)) {
                specifierList.emplace_back("int");
                consumeToken(TokenType::intKeyword);
            }
            else if (matchToken(TokenType::longKeyword)) {
                specifierList.emplace_back("long");
                consumeToken(TokenType::longKeyword);
            }
        }
        consumeToken(TokenType::CloseParenthesis);
        auto targetType = parseType(specifierList);
        auto innerExpr = parseFactor();
        return std::make_shared<CastExpression>(std::move(targetType),
                                                std::move(innerExpr));
    }
    else if (matchToken(TokenType::Tilde)) {
        auto tildeToken = consumeToken(TokenType::Tilde);
        auto innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(tildeToken.value,
                                                 std::move(innerExpr));
    }
    else if (matchToken(TokenType::Minus)) {
        auto minusToken = consumeToken(TokenType::Minus);
        auto innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(minusToken.value,
                                                 std::move(innerExpr));
    }
    else if (matchToken(TokenType::LogicalNot)) {
        auto notToken = consumeToken(TokenType::LogicalNot);
        auto innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(notToken.value,
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
            throw std::runtime_error(
                "Malformed factor: missing closing parenthesis.");
        }
    }
    else {
        std::stringstream msg;
        msg << "Malformed factor: unexpected token: " << tokens[current].value;
        msg << " of type " << tokenTypeToString(tokens[current].type);
        throw std::runtime_error(msg.str());
    }
}

std::shared_ptr<Constant> Parser::parseConstant() {
    auto constantValue = std::stoll(tokens[current].value);
    if (constantValue > pow(2, 63) - 1) {
        throw std::runtime_error(
            "Constant is too large to represent as an int or long");
    }
    else if (tokens[current].type == TokenType::IntConstant) {
        if (constantValue <= pow(2, 31) - 1) {
            consumeToken(TokenType::IntConstant);
            return std::make_shared<ConstantInt>(constantValue);
        }
        else {
            consumeToken(TokenType::IntConstant);
            return std::make_shared<ConstantLong>(constantValue);
        }
    }
    else {
        consumeToken(TokenType::LongConstant);
        return std::make_shared<ConstantLong>(constantValue);
    }
}

std::shared_ptr<Expression> Parser::parseExpression(int minPrecedence) {
    // Parse the left operand of the expression.
    std::shared_ptr<Expression> left = parseFactor();
    // While the next otkn is a binary operator and has a precedence greater
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
            left = std::make_shared<AssignmentExpression>(std::move(left),
                                                          std::move(right));
        }
        else if (matchToken(TokenType::QuestionMark)) {
            auto questionMarkToken = consumeToken(TokenType::QuestionMark);
            auto middle = parseConditionalMiddle();
            auto right = parseExpression(getPrecedence(questionMarkToken));
            left = std::make_shared<ConditionalExpression>(
                std::move(left), std::move(middle), std::move(right));
        }
        // Otherwise, the next token is (should be) a binary operator.
        else {
            auto binOpToken = consumeToken(tokens[current].type);
            if (!(matchToken(TokenType::IntConstant) ||
                  matchToken(TokenType::LongConstant) ||
                  matchToken(TokenType::Tilde) ||
                  matchToken(TokenType::Minus) ||
                  matchToken(TokenType::LogicalNot) ||
                  matchToken(TokenType::OpenParenthesis) ||
                  matchToken(TokenType::Identifier))) {
                std::stringstream msg;
                msg << "Malformed expression: binary operator "
                    << binOpToken.value
                    << " is not followed by a valid operand.";
                throw std::runtime_error(msg.str());
            }
            std::shared_ptr<Expression> right =
                parseExpression(getPrecedence(binOpToken) + 1);
            left = std::make_shared<BinaryExpression>(
                std::move(left), binOpToken.value, std::move(right));
        }
    }
    return left;
}

std::shared_ptr<Expression> Parser::parseConditionalMiddle() {
    // Note: The question mark token has already been consumed in the caller
    // (some `parseExpression` function call). Parse the middle expression.
    auto middle = parseExpression(0);
    // Consume the colon token.
    consumeToken(TokenType::Colon);
    return middle;
}

std::pair<std::shared_ptr<Type>, std::shared_ptr<StorageClass>>
Parser::parseTypeAndStorageClass(
    const std::vector<std::string> &specifierList) {
    std::vector<std::string> types;
    std::vector<std::string> storageClasses;
    for (const auto &specifier : specifierList) {
        if (specifier == "int" || specifier == "long") {
            types.emplace_back(specifier);
        }
        else {
            storageClasses.emplace_back(specifier);
        }
    }
    auto type = parseType(types);
    std::shared_ptr<StorageClass> storageClass;
    if (storageClasses.size() > 1) {
        throw std::runtime_error("Invalid storage class (specifier)");
    }
    if (storageClasses.size() == 1) {
        storageClass = parseStorageClass(storageClasses[0]);
    }
    else {
        storageClass = nullptr;
    }
    return std::make_pair(type, storageClass);
}

std::shared_ptr<Type>
Parser::parseType(const std::vector<std::string> &specifierList) {
    if (specifierList.size() == 1 && specifierList[0] == "int") {
        return std::make_shared<IntType>();
    }
    else if ((specifierList.size() == 2) &&
             ((specifierList[0] == "int" && specifierList[1] == "long") ||
              (specifierList[0] == "long" && specifierList[1] == "int"))) {
        return std::make_shared<LongType>();
    }
    else if (specifierList.size() == 1 && specifierList[0] == "long") {
        return std::make_shared<LongType>();
    }
    else {
        std::stringstream msg;
        msg << "Invalid type specifier of size " << specifierList.size()
            << ": ";
        for (const auto &specifier : specifierList) {
            msg << specifier << " ";
        }
        throw std::runtime_error(msg.str());
    }
}

std::shared_ptr<StorageClass>
Parser::parseStorageClass(const std::string &specifier) {
    if (specifier == "static") {
        return std::make_shared<StaticStorageClass>();
    }
    else if (specifier == "extern") {
        return std::make_shared<ExternStorageClass>();
    }
    else {
        throw std::runtime_error("Invalid storage class (specifier)");
    }
}

int Parser::getPrecedence(const Token &token) {
    if (precedenceMap.find(token.type) != precedenceMap.end()) {
        return precedenceMap[token.type];
    }
    return -1;
}
} // namespace AST
