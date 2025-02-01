#ifndef FRONTEND_LEXER_H
#define FRONTEND_LEXER_H

#include <iostream>
#include <regex>
#include <string>
#include <vector>

// Regular expressions for the different token types.
// The caret symbol (^) matches the start of a line or a string.
// It asserts that the current position in the string is at the beginning of a
// line or the string.
const std::regex identifier_regex(R"(^[a-zA-Z_]\w*\b)");
const std::regex longIntConstant_regex(R"(^[0-9]+[lL]\b)");
const std::regex intConstant_regex(R"(^[0-9]+\b)");
const std::regex intKeyword_regex(R"(^int\b)");
const std::regex voidKeyword_regex(R"(^void\b)");
const std::regex returnKeyword_regex(R"(^return\b)");
const std::regex ifKeyword_regex(R"(^if\b)");
const std::regex elseKeyword_regex(R"(^else\b)");
const std::regex doKeyword_regex(R"(^do\b)");
const std::regex whileKeyword_regex(R"(^while\b)");
const std::regex forKeyword_regex(R"(^for\b)");
const std::regex breakKeyword_regex(R"(^break\b)");
const std::regex continueKeyword_regex(R"(^continue\b)");
const std::regex staticKeyword_regex(R"(^static\b)");
const std::regex externKeyword_regex(R"(^extern\b)");
const std::regex longKeyword_regex(R"(^long\b)");
const std::regex comma_regex(R"(^\,)");
const std::regex questionMark_regex(R"(^\?)");
const std::regex colon_regex(R"(^\:)");
const std::regex assign_regex(R"(^=)");
const std::regex openParenthesis_regex(R"(^\()");
const std::regex closeParenthesis_regex(R"(^\))");
const std::regex openBrace_regex(R"(^\{)");
const std::regex closeBrace_regex(R"(^\})");
const std::regex semicolon_regex(R"(^;)");
const std::regex tilde_regex(R"(^~)");
const std::regex twoHyphen_regex(R"(^--)");
const std::regex plus_regex(R"(^\+)");
const std::regex minus_regex(R"(^-)"); // This is also used as a hyphen regex.
const std::regex multiply_regex(R"(^\*)");
const std::regex divide_regex(R"(^\/)");
const std::regex modulo_regex(R"(^%)");
const std::regex logicalNot_regex(R"(^!)");
const std::regex logicalAnd_regex(R"(^&&)");
const std::regex logicalOr_regex(R"(^\|\|)");
const std::regex equal_regex(R"(^==)");
const std::regex notEqual_regex(R"(^!=)");
const std::regex lessThanOrEqual_regex(R"(^<=)");
const std::regex greaterThanOrEqual_regex(R"(^>=)");
const std::regex lessThan_regex(R"(^<)");
const std::regex greaterThan_regex(R"(^>)");
const std::regex singleLineComment_regex(R"(^\/\/[^\n]*\n?)");
const std::regex multiLineComment_regex(R"(^\/\*[\s\S]*?\*\/)");
const std::regex stringLiteral_regex(R"(^\".*?\"|^\'.*?\')");
const std::regex preprocessorDirective_regex(R"(^#\w+)");

// Define token types as an enum class.
enum class TokenType {
    Identifier,
    LongIntConstant,
    IntConstant,
    intKeyword,
    voidKeyword,
    returnKeyword,
    ifKeyword,
    elseKeyword,
    doKeyword,
    whileKeyword,
    forKeyword,
    breakKeyword,
    continueKeyword,
    staticKeyword,
    externKeyword,
    longKeyword,
    Comma,
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

#endif // FRONTEND_LEXER_H
