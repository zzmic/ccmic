

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "expression.h"
#include "function.h"
#include "lexer.h"
#include "program.h"
#include "statement.h"

namespace AST {
class Parser {
  public:
    Parser(const std::vector<Token> &tokens) : tokens_(tokens), current_(0) {}
    std::shared_ptr<Program> parse();

  private:
    const std::vector<Token> &tokens_;
    std::size_t current_;

    bool matchToken(TokenType type);
    Token consumeToken(TokenType type);
    void expectToken(TokenType type);
    std::shared_ptr<Function> parseFunction();
    std::shared_ptr<Statement> parseStatement();
    std::shared_ptr<Expression> parseExpression();
    // Expression *parseBinaryExpression(int precedence);
    // int getPrecedence(const Token &token);
};
} // Namespace AST

#endif // PARSER_H
