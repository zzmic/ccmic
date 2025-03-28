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
#include <unordered_map>

namespace AST {
class Parser {
  public:
    Parser(const std::vector<Token> &tokens);
    std::shared_ptr<Program> parse();

  private:
    const std::vector<Token> &tokens;
    size_t current;
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
    std::shared_ptr<Declaration> parseDeclaration();
    void parseTypeSpecifiersInParameters(
        std::shared_ptr<std::vector<std::string>> &parameters);
    std::shared_ptr<BlockItem> parseBlockItem();
    std::shared_ptr<Block> parseBlock();
    std::shared_ptr<ForInit> parseForInit();
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Expression> parseFactor();
    std::shared_ptr<Constant> parseConstant();
    std::shared_ptr<Expression> parseExpression(int minPrecedence = 0);
    std::shared_ptr<Expression> parseConditionalMiddle();
    std::pair<std::shared_ptr<Type>, std::shared_ptr<StorageClass>>
    parseTypeAndStorageClass(std::vector<std::string> &specifierList);
    std::shared_ptr<Type> parseType(std::vector<std::string> &specifierList);
    std::shared_ptr<StorageClass> parseStorageClass(std::string &specifier);
    int getPrecedence(const Token &token);
};
} // Namespace AST

#endif // FRONTEND_PARSER_H
