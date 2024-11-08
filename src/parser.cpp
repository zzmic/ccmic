#include "parser.h"
#include <sstream>

namespace AST {
bool Parser::matchToken(TokenType type) {
    // If the current index is less than the number of tokens and the current
    // token is of the expected type, return true. Otherwise, return false.
    if (current_ < tokens_.size() && tokens_[current_].type == type) {
        return true;
    }
    return false;
}

Token Parser::consumeToken(TokenType type) {
    // If the current index is less than the number of tokens and the current
    // token is of the expected type, consume the token, increment the
    // current index, and return the token. Otherwise, throw an error.
    if (current_ < tokens_.size() && tokens_[current_].type == type) {
        return tokens_[current_++];
    }
    std::stringstream msg;
    msg << "Expect token of type " << tokenTypeToString(type) << " but found "
        << tokens_[current_].value;
    msg << " of type " << tokenTypeToString(tokens_[current_].type);
    throw std::runtime_error(msg.str());
}

void Parser::expectToken(TokenType type) {
    // If the current token is not of the expected type, throw an error.
    // Otherwise, consume the token.
    if (!matchToken(type)) {
        std::stringstream msg;
        msg << "Expect token of type " << tokenTypeToString(type)
            << " but found " << tokens_[current_].value;
        msg << " of type " << tokenTypeToString(tokens_[current_].type);
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
    expectToken(TokenType::OpenBrace);
    std::shared_ptr<Statement> functionBody = parseStatement();
    expectToken(TokenType::CloseBrace);

    if (current_ < tokens_.size()) {
        std::stringstream msg;
        msg << "Unexpected token: " << tokens_[current_].value;
        msg << " of type " << tokenTypeToString(tokens_[current_].type);
        msg << " since the token search should be saturated";
        throw std::runtime_error(msg.str());
    }

    return std::shared_ptr<Function>(
        new Function(functionNameToken.value, functionBody));
}

std::shared_ptr<Statement> Parser::parseStatement() {
    if (matchToken(TokenType::returnKeyword)) {
        consumeToken(TokenType::returnKeyword);
        std::shared_ptr<Expression> value = parseExpression();
        expectToken(TokenType::Semicolon);
        return std::shared_ptr<Statement>(new ReturnStatement(value));
    }
    std::stringstream msg;
    msg << "Unexpected statement: " << tokens_[current_].value;
    msg << " of type " << tokenTypeToString(tokens_[current_].type);
    throw std::runtime_error(msg.str());
}

std::shared_ptr<Expression> Parser::parseExpression() {
    if (matchToken(TokenType::Constant)) {
        Token constantToken = consumeToken(TokenType::Constant);
        return std::shared_ptr<Expression>(
            new ConstantExpression(std::stoi(constantToken.value)));
    }
    else if (matchToken(TokenType::Tilde)) {
        Token tildeToken = consumeToken(TokenType::Tilde);
        std::shared_ptr<Expression> innerExpr = parseExpression();
        return std::shared_ptr<UnaryExpression>(
            new UnaryExpression(tildeToken.value, innerExpr));
    }
    else if (matchToken(TokenType::Minus)) {
        Token minusToken = consumeToken(TokenType::Minus);
        std::shared_ptr<Expression> innerExpr = parseExpression();
        return std::shared_ptr<UnaryExpression>(
            new UnaryExpression(minusToken.value, innerExpr));
    }
    else if (matchToken(TokenType::OpenParenthesis)) {
        consumeToken(TokenType::OpenParenthesis);
        std::shared_ptr<Expression> innerExpr = parseExpression();
        if (matchToken(TokenType::CloseParenthesis)) {
            consumeToken(TokenType::CloseParenthesis);
            return innerExpr;
        }
        else {
            std::stringstream msg;
            msg << "Malformed expression";
            throw std::runtime_error(msg.str());
        }
    }
    else {
        std::stringstream msg;
        msg << "Malformed expression";
        throw std::runtime_error(msg.str());
    }
}

// // TODO(zzmic): Make this function generic to handle either binary or primary
// // expressions instead of "branching" to parse eiher a binary or primary
// // expression.
// Expression *Parser::parseBinaryExpression(int precedence) {
//     // Parse the left side of the expression first.
//     Expression *left = parseConstantExpression();

//     // While the current_ token is a binary operator and the precedence of
//     // the current_ token is greater than or equal to the precedence of the
//     // previo token, ...
//     while (current_ < tokens_.size()) {
//         // Get the current_ token and its precedence.
//         Token token = tokens_[current_];
//         int tokenPrecedence = getPrecedence(token);

//         // If the token precedence is less than the precedence of the
//         // current token, break.
//         if (tokenPrecedence < precedence) {
//             break;
//         }

//         // Consume the operator token and parse the right side of the
//         // expression.
//         // `(tokenPrecedence+1)` helps to ensure that any operator with a
//         // higher or equivalent precedence is handled first and the parser
//         correctly
//         // nests expressions, maintaining the correct order of operations.
//         consumeToken(token.type);
//         Expression *right = parseBinaryExpression(tokenPrecedence + 1);
//         // Create a new binary expression with the left side, operator, and
//         // right side, and assign it to the left side.
//         left = new BinaryExpression(left, token.value, right);
//     }

//     return left;
// }

// int Parser::getPrecedence(const Token &token) {
//     // If the token is a plus or minus operator, return 1.
//     if (token.type == TokenType::Plus || token.type == TokenType::Minus) {
//         return 1;
//     }
//     // If the token is a multiply or divide operator, return 2 (higher than
//     // plus or minus' precedence).
//     if (token.type == TokenType::Multiply || token.type == TokenType::Divide)
//     {
//         return 2;
//     }
//     return 0;
// }

std::shared_ptr<Program> Parser::parse() {
    std::shared_ptr<Function> function = parseFunction();
    return std::shared_ptr<Program>(new Program(function));
}
} // Namespace AST
