#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "declaration.h"
#include "expression.h"
#include "function.h"
#include "lexer.h"
#include "program.h"
#include "statement.h"
#include <unordered_map>

namespace AST {
class Parser {
  public:
    Parser(const std::vector<Token> &tokens);
    std::shared_ptr<Program> parse();

  private:
    const std::vector<Token> &tokens;
    std::size_t current;
    std::unordered_map<TokenType, int> precedenceMap = {
        {TokenType::Assign, 1},       {TokenType::QuestionMark, 3},
        {TokenType::LogicalOr, 5},    {TokenType::LogicalAnd, 10},
        {TokenType::Equal, 30},       {TokenType::NotEqual, 30},
        {TokenType::LessThan, 35},    {TokenType::LessThanOrEqual, 35},
        {TokenType::GreaterThan, 35}, {TokenType::GreaterThanOrEqual, 35},
        {TokenType::Plus, 45},        {TokenType::Minus, 45},
        {TokenType::Multiply, 50},    {TokenType::Divide, 50},
        {TokenType::Modulo, 50},
    };

    bool matchToken(TokenType type);
    Token consumeToken(TokenType type);
    void expectToken(TokenType type);
    std::shared_ptr<Function> parseFunction();
    std::shared_ptr<BlockItem> parseBlockItem();
    std::shared_ptr<Block> parseBlock();
    std::shared_ptr<Declaration> parseDeclaration();
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Expression> parseFactor();
    std::shared_ptr<Expression> parseExpression(int minPrecedence = 0);
    std::shared_ptr<Expression> parseConditionalMiddle();
    int getPrecedence(const Token &token);
};
} // Namespace AST

#endif // PARSER_H
