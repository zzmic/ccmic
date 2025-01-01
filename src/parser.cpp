#include "parser.h"
#include <sstream>

namespace AST {
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

std::shared_ptr<Program> Parser::parse() {
    auto function = parseFunction();
    return std::make_shared<Program>(std::move(function));
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

std::shared_ptr<Function> Parser::parseFunction() {
    expectToken(TokenType::intKeyword);
    auto functionNameToken = consumeToken(TokenType::Identifier);
    expectToken(TokenType::OpenParenthesis);
    expectToken(TokenType::voidKeyword);
    expectToken(TokenType::CloseParenthesis);

    auto functionBody = parseBlock();

    if (current < tokens.size()) {
        std::stringstream msg;
        msg << "Malformed function: unexpected token: "
            << tokens[current].value;
        msg << " of type " << tokenTypeToString(tokens[current].type);
        msg << " since the token search should be saturated";
        throw std::runtime_error(msg.str());
    }

    return std::make_shared<Function>(functionNameToken.value,
                                      std::move(functionBody));
}

std::shared_ptr<BlockItem> Parser::parseBlockItem() {
    if (matchToken(TokenType::intKeyword)) {
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
    expectToken(TokenType::intKeyword);
    auto variableNameToken = consumeToken(TokenType::Identifier);
    if (matchToken(TokenType::Assign)) {
        consumeToken(TokenType::Assign);
        auto expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<Declaration>(variableNameToken.value,
                                             std::move(expr));
    }
    else {
        expectToken(TokenType::Semicolon);
        return std::make_shared<Declaration>(variableNameToken.value);
    }
}

std::shared_ptr<ForInit> Parser::parseForInit() {
    if (matchToken(TokenType::intKeyword)) {
        auto declaration = parseDeclaration();
        return std::make_shared<InitDecl>(std::move(declaration));
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
    if (matchToken(TokenType::Constant)) {
        auto constantToken = consumeToken(TokenType::Constant);
        return std::make_shared<ConstantExpression>(
            std::stoi(constantToken.value));
    }
    else if (matchToken(TokenType::Identifier)) {
        auto identifierToken = consumeToken(TokenType::Identifier);
        return std::make_shared<VariableExpression>(identifierToken.value);
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
        msg << "\n" << tokens[current - 1].value;
        msg << "\n" << tokens[current - 2].value;
        msg << "\n" << tokens[current - 3].value;
        msg << "\n" << tokens[current - 4].value;
        throw std::runtime_error(msg.str());
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
            if (!(matchToken(TokenType::Constant) ||
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

int Parser::getPrecedence(const Token &token) {
    if (precedenceMap.find(token.type) != precedenceMap.end()) {
        return precedenceMap[token.type];
    }
    return -1;
}
} // Namespace AST
