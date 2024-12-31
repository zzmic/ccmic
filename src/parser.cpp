#include "parser.h"
#include <sstream>

namespace AST {
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

std::shared_ptr<Program> Parser::parse() {
    std::shared_ptr<Function> function = parseFunction();
    return std::make_shared<Program>(function);
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
    Token functionNameToken = consumeToken(TokenType::Identifier);
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

    return std::make_shared<Function>(functionNameToken.value, functionBody);
}

std::shared_ptr<BlockItem> Parser::parseBlockItem() {
    if (matchToken(TokenType::intKeyword)) {
        auto declaration = parseDeclaration();
        return std::make_shared<DBlockItem>(declaration);
    }
    else {
        auto statement = parseStatement();
        return std::make_shared<SBlockItem>(statement);
    }
}

std::shared_ptr<Block> Parser::parseBlock() {
    expectToken(TokenType::OpenBrace);
    auto blockItems =
        std::make_shared<std::vector<std::shared_ptr<BlockItem>>>();
    while (!matchToken(TokenType::CloseBrace)) {
        auto nextBlockItem = parseBlockItem();
        blockItems->emplace_back(nextBlockItem);
    }
    expectToken(TokenType::CloseBrace);
    return std::make_shared<Block>(blockItems);
}

std::shared_ptr<Declaration> Parser::parseDeclaration() {
    expectToken(TokenType::intKeyword);
    Token variableNameToken = consumeToken(TokenType::Identifier);
    if (matchToken(TokenType::Assign)) {
        consumeToken(TokenType::Assign);
        std::shared_ptr<Expression> expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<Declaration>(variableNameToken.value, expr);
    }
    else {
        expectToken(TokenType::Semicolon);
        return std::make_shared<Declaration>(variableNameToken.value);
    }
}

std::shared_ptr<ForInit> Parser::parseForInit() {
    if (matchToken(TokenType::intKeyword)) {
        auto declaration = parseDeclaration();
        return std::make_shared<InitDecl>(declaration);
    }
    else {
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
            return std::make_shared<InitExpr>();
        }
        else {
            std::shared_ptr<Expression> expr = parseExpression(0);
            expectToken(TokenType::Semicolon);
            return std::make_shared<InitExpr>(expr);
        }
    }
}

std::shared_ptr<Statement> Parser::parseStatement() {
    // Parse a return statement.
    if (matchToken(TokenType::returnKeyword)) {
        consumeToken(TokenType::returnKeyword);
        std::shared_ptr<Expression> expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<ReturnStatement>(expr);
    }
    // Parse a null statement.
    else if (matchToken(TokenType::Semicolon)) {
        consumeToken(TokenType::Semicolon);
        return std::make_shared<NullStatement>();
    }
    // Parse an if statement.
    else if (matchToken(TokenType::ifKeyword)) {
        consumeToken(TokenType::ifKeyword);
        expectToken(TokenType::OpenParenthesis);
        std::shared_ptr<Expression> condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        std::shared_ptr<Statement> thenStatement = parseStatement();
        if (matchToken(TokenType::elseKeyword)) {
            consumeToken(TokenType::elseKeyword);
            std::shared_ptr<Statement> elseStatement = parseStatement();
            return std::make_shared<IfStatement>(condition, thenStatement,
                                                 elseStatement);
        }
        return std::make_shared<IfStatement>(condition, thenStatement);
    }
    // Parse a compound statement.
    else if (matchToken(TokenType::OpenBrace)) {
        return std::make_shared<CompoundStatement>(parseBlock());
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
    // Parse a while statement.
    else if (matchToken(TokenType::whileKeyword)) {
        consumeToken(TokenType::whileKeyword);
        expectToken(TokenType::OpenParenthesis);
        std::shared_ptr<Expression> condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        std::shared_ptr<Statement> body = parseStatement();
        return std::make_shared<WhileStatement>(condition, body);
    }
    // Parse a do-while statement.
    else if (matchToken(TokenType::doKeyword)) {
        consumeToken(TokenType::doKeyword);
        std::shared_ptr<Statement> body = parseStatement();
        expectToken(TokenType::whileKeyword);
        expectToken(TokenType::OpenParenthesis);
        std::shared_ptr<Expression> condition = parseExpression(0);
        expectToken(TokenType::CloseParenthesis);
        expectToken(TokenType::Semicolon);
        return std::make_shared<DoWhileStatement>(condition, body);
    }
    // Parse a for statement.
    else if (matchToken(TokenType::forKeyword)) {
        consumeToken(TokenType::forKeyword);
        expectToken(TokenType::OpenParenthesis);
        std::shared_ptr<ForInit> init = parseForInit();
        std::optional<std::shared_ptr<Expression>> optCondition;
        std::optional<std::shared_ptr<Expression>> optPost;
        if (matchToken(TokenType::Semicolon)) {
            consumeToken(TokenType::Semicolon);
        }
        else {
            std::shared_ptr<Expression> condition = parseExpression(0);
            optCondition = std::make_optional(condition);
            expectToken(TokenType::Semicolon);
        }
        if (matchToken(TokenType::CloseParenthesis)) {
            consumeToken(TokenType::CloseParenthesis);
        }
        else {
            std::shared_ptr<Expression> post = parseExpression(0);
            optPost = std::make_optional(post);
            expectToken(TokenType::CloseParenthesis);
        }
        std::shared_ptr<Statement> body = parseStatement();
        return std::make_shared<ForStatement>(init, optCondition, optPost,
                                              body);
    }
    // Parse an expression statement.
    else {
        std::shared_ptr<Expression> expr = parseExpression(0);
        expectToken(TokenType::Semicolon);
        return std::make_shared<ExpressionStatement>(expr);
    }
    std::stringstream msg;
    msg << "Malformed statement: unexpected token: " << tokens[current].value;
    msg << " of type " << tokenTypeToString(tokens[current].type);
    throw std::runtime_error(msg.str());
}

std::shared_ptr<Expression> Parser::parseFactor() {
    if (matchToken(TokenType::Constant)) {
        Token constantToken = consumeToken(TokenType::Constant);
        return std::make_shared<ConstantExpression>(
            std::stoi(constantToken.value));
    }
    else if (matchToken(TokenType::Identifier)) {
        Token identifierToken = consumeToken(TokenType::Identifier);
        return std::make_shared<VariableExpression>(identifierToken.value);
    }
    else if (matchToken(TokenType::Tilde)) {
        Token tildeToken = consumeToken(TokenType::Tilde);
        std::shared_ptr<Expression> innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(tildeToken.value, innerExpr);
    }
    else if (matchToken(TokenType::Minus)) {
        Token minusToken = consumeToken(TokenType::Minus);
        std::shared_ptr<Expression> innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(minusToken.value, innerExpr);
    }
    else if (matchToken(TokenType::LogicalNot)) {
        Token notToken = consumeToken(TokenType::LogicalNot);
        std::shared_ptr<Expression> innerExpr = parseFactor();
        return std::make_shared<UnaryExpression>(notToken.value, innerExpr);
    }
    else if (matchToken(TokenType::OpenParenthesis)) {
        consumeToken(TokenType::OpenParenthesis);
        std::shared_ptr<Expression> innerExpr = parseExpression(0);
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
            Token assignToken = consumeToken(TokenType::Assign);
            auto right = parseExpression(getPrecedence(assignToken));
            left = std::make_shared<AssignmentExpression>(left, right);
        }
        else if (matchToken(TokenType::QuestionMark)) {
            Token questionMarkToken = consumeToken(TokenType::QuestionMark);
            auto middle = parseConditionalMiddle();
            auto right = parseExpression(getPrecedence(questionMarkToken));
            left = std::make_shared<ConditionalExpression>(left, middle, right);
        }
        // Otherwise, the next token is (should be) a binary operator.
        else {
            Token binOpToken = consumeToken(tokens[current].type);
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
            left = std::make_shared<BinaryExpression>(left, binOpToken.value,
                                                      right);
        }
    }
    return left;
}

std::shared_ptr<Expression> Parser::parseConditionalMiddle() {
    // Note: The question mark token has already been consumed in the caller
    // (some `parseExpression` function call). Parse the middle expression.
    std::shared_ptr<Expression> middle = parseExpression(0);
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
