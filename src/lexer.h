#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <regex>
#include <string>
#include <vector>

// Define token types as an enum class.
enum class TokenType {
    Identifier,
    Constant,
    intKeyword,
    voidKeyword,
    returnKeyword,
    ifKeyword,
    elseKeyword,
    QuestionMark,
    Colon,
    Assign,
    OpenParenthesis,
    CloseParenthesis,
    OpenBrace,
    CloseBrace,
    Semicolon,
    Tilde,
    TwoHyphen,
    Plus,
    Minus, // This is also used as a hyphen.
    Multiply,
    Divide,
    Modulo,
    LogicalNot,
    LogicalAnd,
    LogicalOr,
    Equal,
    NotEqual,
    LessThanOrEqual,
    GreaterThanOrEqual,
    LessThan,
    GreaterThan,
    SingleLineComment,
    MultiLineComment,
    StringLiteral,
    PreprocessorDirective,
    Invalid
};

// Define a struct to represent a token.
struct Token {
    TokenType type;
    std::string value;
};

Token matchToken(const std::string &input);
std::vector<Token> lexer(const std::string &input);
void printTokens(const std::vector<Token> &tokens);
std::string tokenTypeToString(TokenType type);

#endif // LEXER_H
