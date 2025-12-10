#ifndef FRONTEND_LEXER_H
#define FRONTEND_LEXER_H

#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

/**
 * Regular expressions for different token types.
 *
 * The caret symbol (^) matches the start of a line or a string.
 * It asserts that the current position in the string is at the beginning of a
 * line or the string.
 */
const std::regex identifier_regex(R"(^[a-zA-Z_]\w*\b)");
const std::regex LongConstant_regex(R"(^[0-9]+[lL]\b)");
const std::regex intConstant_regex(R"(^[0-9]+\b)");
const std::regex intKeyword_regex(R"(^int\b)");
const std::regex longKeyword_regex(R"(^long\b)");
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
const std::regex
    minus_regex(R"(^-)"); // This is also referred to as a hyphen regex.
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

/**
 * Enumeration representing different types of tokens.
 *
 * Each enumerator has a local name scope to the enumeration, and their
 * values are not implicitly converted to integers or other types.
 * Reference:
 * https://stackoverflow.com/questions/18335861/why-is-enum-class-considered-safer-to-use-than-plain-enum.
 */
enum class TokenType {
    Identifier,
    LongConstant,
    IntConstant,
    intKeyword,
    longKeyword,
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
    SingleLineComment,
    MultiLineComment,
    StringLiteral,
    PreprocessorDirective,
    Invalid
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
