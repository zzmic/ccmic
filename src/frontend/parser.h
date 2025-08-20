#ifndef FRONTEND_PARSER_H
#define FRONTEND_PARSER_H

#include "ast.h"
#include "declaration.h"
#include "expression.h"
#include "function.h"
#include "lexer.h"
#include "program.h"
#include "statement.h"
#include "type.h"
#include <memory>
#include <unordered_map>

namespace AST {
class Parser {
  public:
    explicit Parser(const std::vector<Token> &tokens);
    std::unique_ptr<Program> parse();

  private:
    const std::vector<Token> &tokens;
    // The current token being processed.
    size_t current;
    // A map that maps token types to their precedence(s).
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
    std::unique_ptr<Declaration> parseDeclaration();
    void parseTypeSpecifiersInParameters(std::vector<std::string> &parameters);
    std::unique_ptr<BlockItem> parseBlockItem();
    Block *parseBlock();
    std::unique_ptr<ForInit> parseForInit();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Constant> parseConstant();
    std::unique_ptr<Expression> parseExpression(int minPrecedence = 0);
    std::unique_ptr<Expression> parseConditionalMiddle();
    std::pair<std::unique_ptr<Type>, std::unique_ptr<StorageClass>>
    parseTypeAndStorageClass(const std::vector<std::string> &specifierList);
    std::unique_ptr<Type>
    parseType(const std::vector<std::string> &specifierList);
    std::unique_ptr<StorageClass>
    parseStorageClass(const std::string &specifier);
    int getPrecedence(const Token &token) const;
};
} // Namespace AST

#endif // FRONTEND_PARSER_H
