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
/**
 * Class representing a parser that builds an abstract syntax tree (AST) from a
 * sequence of tokens.
 */
class Parser {
  public:
    /**
     * Constructor for the Parser class.
     *
     * @param tokens A vector of tokens to be parsed.
     */
    Parser(const std::vector<Token> &tokens);

    /**
     * Parse the tokens and return the root of the AST (`Program` node).
     *
     * @return The root of the AST (as a shared pointer to a `Program` node).
     */
    std::shared_ptr<Program> parse();

  private:
    /**
     * The list of tokens to be parsed.
     */
    const std::vector<Token> &tokens;

    /**
     * The current token being processed.
     */
    size_t current;

    /**
     * The map that maps token types to their precedence(s).
     */
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

    /**
     * Check if the current token matches the expected type.
     *
     * @param type The expected token type.
     */
    bool matchToken(TokenType type);

    /**
     * Consume the current token if it matches the expected type, otherwise
     * throw an error.
     *
     * @param type The expected token type.
     */
    Token consumeToken(TokenType type);

    /**
     * Expect the current token to be of the specified type, otherwise throw
     * an error.
     *
     * @param type The expected token type.
     */
    void expectToken(TokenType type);

    /**
     * Parse a declaration.
     *
     * @return The declaration node.
     */
    std::shared_ptr<Declaration> parseDeclaration();

    /**
     * Parse type specifiers in function parameters.
     *
     * @param parameters The list of parameter specifiers.
     */
    std::vector<std::string> parseTypeSpecifiersInParameters();

    /**
     * Parse a block item.
     *
     * @return The block item node.
     */
    std::shared_ptr<BlockItem> parseBlockItem();

    /**
     * Parse a block.
     *
     * @return The block node.
     */
    std::shared_ptr<Block> parseBlock();

    /**
     * Parse a function definition.
     *
     * @return The function definition node.
     */
    std::shared_ptr<ForInit> parseForInit();

    /**
     * Parse a statement.
     *
     * @return The statement node.
     */
    std::shared_ptr<Statement> parseStatement();

    /**
     * Parse a factor.
     *
     * @return The expression node representing the factor.
     */
    std::shared_ptr<Expression> parseFactor();

    /**
     * Parse a constant.
     *
     * @return The constant node.
     */
    std::shared_ptr<Constant> parseConstant();

    /**
     * Parse an expression with the given minimum precedence.
     *
     * @param minPrecedence The minimum precedence for parsing the expression.
     * @return The expression node.
     */
    std::shared_ptr<Expression> parseExpression(int minPrecedence = 0);

    /**
     * Parse the middle part of a conditional expression (between '?' and ':').
     *
     * @return The expression node representing the
     * conditional middle.
     */
    std::shared_ptr<Expression> parseConditionalMiddle();

    /**
     * Parse a type and storage class from a list of specifiers.
     *
     * @param specifierList A vector of specifier strings.
     * @return A pair consisting of (a shared pointer to) the type and a (shared
     * pointer) to the storage class.
     */
    std::pair<std::shared_ptr<Type>, std::shared_ptr<StorageClass>>
    parseTypeAndStorageClass(const std::vector<std::string> &specifierList);

    /**
     * Parse a type from a list of specifiers.
     *
     * @param specifierList A vector of specifier strings.
     * @return The type.
     */
    std::shared_ptr<Type>
    parseType(const std::vector<std::string> &specifierList);

    /**
     * Parse a storage class from a specifier string.
     *
     * @param specifier A specifier string.
     * @return The storage class.
     */
    std::shared_ptr<StorageClass>
    parseStorageClass(const std::string &specifier);

    /**
     * Get the precedence of a token.
     *
     * @param token The token whose precedence is to be retrieved.
     * @return The precedence of the token.
     */
    int getPrecedence(const Token &token);
};
} // Namespace AST

#endif // FRONTEND_PARSER_H
