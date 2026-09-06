#ifndef FRONTEND_LEXER_H
#define FRONTEND_LEXER_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

/**
 * Enumeration representing different types of tokens.
 *
 * Each enumerator has a local name scope to the enumeration, and their
 * values are not implicitly converted to integers or other types.
 * Reference:
 * https://stackoverflow.com/questions/18335861/why-is-enum-class-considered-safer-to-use-than-plain-enum.
 */
enum class TokenType : std::uint8_t {
    Identifier,
    LongConstant,
    IntConstant,
    intKeyword,
    longKeyword,
    signedKeyword,
    unsignedKeyword,
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
    Minus, // This is also referred to as a hyphen.
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
    UnsignedIntegerConstant,
    UnsignedLongIntegerConstant,
    SingleLineComment,
    MultiLineComment,
    StringLiteral,
    PreprocessorDirective,
    Invalid,
};

/**
 * Structure representing a token with its type and value.
 */
struct Token {
    TokenType type;
    std::string value;
};

/**
 * Match a token from the input string.
 *
 * Match the input string against the regular expressions for the different
 * token types and return the token struct, containing the token type and the
 * token value (in string).
 *
 * @param input The input string to match the token from.
 * @return The matched token.
 */
Token matchToken(std::string_view input);

/**
 * Lex the input string into a vector of tokens.
 *
 * @param input The input string to lex.
 * @return A vector of tokens.
 */
std::vector<Token> lexer(std::string_view input);

/**
 * Pretty-print the tokens to the stdout.
 *
 * @param tokens The vector of tokens to print.
 */
void printTokens(const std::vector<Token> &tokens);

/**
 * Convert a token type to its string representation.
 *
 * @param type The TokenType to convert.
 * @return The string representation of the TokenType.
 */
std::string tokenTypeToString(TokenType type);

#endif // FRONTEND_LEXER_H
