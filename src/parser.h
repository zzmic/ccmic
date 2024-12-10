#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
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
    std::unordered_map<TokenType, int> precedenceMap;

    bool matchToken(TokenType type);
    Token consumeToken(TokenType type);
    void expectToken(TokenType type);
    std::shared_ptr<Function> parseFunction();
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Expression> parseFactor();
    std::shared_ptr<Expression> parseExpression(int minPrecedence = 0);
    int getPrecedence(const Token &token);
};
} // Namespace AST

#endif // PARSER_H
