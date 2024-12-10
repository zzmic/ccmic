#include "lexer.h"
#include <sstream>

// Regular expressions for the different token types.
// The caret symbol (^) matches the start of a line or a string.
// It asserts that the current position in the string is at the beginning of a
// line or the string.
const std::regex identifier_regex(R"(^[a-zA-Z_]\w*\b)");
const std::regex constant_regex(R"(^[0-9]+\b)");
const std::regex intKeyword_regex(R"(^int\b)");
const std::regex voidKeyword_regex(R"(^void\b)");
const std::regex returnKeyword_regex(R"(^return\b)");
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
const std::regex singleLineComment_regex(R"(^\/\/[^\n]*\n?)");
const std::regex multiLineComment_regex(R"(^\/\*[\s\S]*?\*\/)");

// Match a token and return its type.
Token matchToken(const std::string &input) {
    // Instantiate the `match_results` class template for matches on string
    // objects.
    std::smatch tokenMatch;

    // Match the input string against the regular expressions for the different
    // token types and return the token struct, containing the token type and
    // the token value (string).
    // Raise up the precedence of token-matching `singleLineComment_regex` and
    // `multiLineComment_regex` to resolve the conflict with the other token
    // matchings (e.g., `multiply_regex`).
    if (std::regex_search(input, tokenMatch, singleLineComment_regex))
        return {TokenType::SingleLineComment, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, multiLineComment_regex))
        return {TokenType::MultiLineComment, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, constant_regex))
        return {TokenType::Constant, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, intKeyword_regex))
        return {TokenType::intKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, voidKeyword_regex))
        return {TokenType::voidKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, returnKeyword_regex))
        return {TokenType::returnKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, openParenthesis_regex))
        return {TokenType::OpenParenthesis, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, closeParenthesis_regex))
        return {TokenType::CloseParenthesis, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, openBrace_regex))
        return {TokenType::OpenBrace, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, closeBrace_regex))
        return {TokenType::CloseBrace, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, semicolon_regex))
        return {TokenType::Semicolon, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, tilde_regex))
        return {TokenType::Tilde, tokenMatch.str()};
    // Enforce the precedence of token-matching `twoHyphen_regex` over
    // `minus_regex` to avoid the conflict with the token matching
    // `minus_regex`.
    else if (std::regex_search(input, tokenMatch, twoHyphen_regex))
        return {TokenType::TwoHyphen, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, plus_regex))
        return {TokenType::Plus, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, minus_regex))
        return {TokenType::Minus, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, multiply_regex))
        return {TokenType::Multiply, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, divide_regex))
        return {TokenType::Divide, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, modulo_regex))
        return {TokenType::Modulo, tokenMatch.str()};
    // Lower down the precedence of token-matching `identifier_regex`
    // to avoid the conflict with the other token matchings (e.g.,
    // `intKeyword_regex`)
    else if (std::regex_search(input, tokenMatch, identifier_regex))
        return {TokenType::Identifier, tokenMatch.str()};
    // If no match is found, return an "invalid" token.
    else
        return {TokenType::Invalid, input};
}

// Tokenize the input.
std::vector<Token> lexer(const std::string &input) {
    std::vector<Token> tokens;
    std::string remaining_input = input;

    // Process the input string until it is empty.
    while (!remaining_input.empty()) {
        // If the input starts with some whitespace, trim the whitespace from
        // the start of the input.
        remaining_input =
            std::regex_replace(remaining_input, std::regex("^\\s+"), "");
        if (remaining_input.empty())
            break;

        // Find the longest match at the start of the input for any regex
        // specified in Table 1-1 (page 9).
        Token token = matchToken(remaining_input);

        // If no match is found, raise an error.
        // Print out the remaining input (intentionally) and exit the program.
        if (token.type == TokenType::Invalid) {
            std::stringstream msg;
            msg << "Invalid token found: " << token.value;
            throw std::runtime_error(msg.str());
        }

        // Skip the token (matching substring) if the token is of type
        // `SingleLineComment` or `MultiLineComment`.
        if (token.type == TokenType::SingleLineComment ||
            token.type == TokenType::MultiLineComment) {
            remaining_input = remaining_input.substr(token.value.size());
            continue;
        }

        // Add the token (matching substring) to the list of tokens except for
        // tokens of types `SingleLineComment` and `MultiLineComment`.
        tokens.push_back(token);

        // Remove the token (matching substring) from the start of the input.
        remaining_input = remaining_input.substr(token.value.size());
    }

    // Return the vector of tokens.
    return tokens;
}

// Print the tokens.
void printTokens(const std::vector<Token> &tokens) {
    // For each token, print the token type (converted from each token type to
    // its corresponding string) and the token value.
    for (const auto &token : tokens) {
        std::string typeStr;
        switch (token.type) {
        case TokenType::Identifier:
            typeStr = "Identifier";
            break;
        case TokenType::Constant:
            typeStr = "Constant";
            break;
        case TokenType::intKeyword:
            typeStr = "intKeyword";
            break;
        case TokenType::voidKeyword:
            typeStr = "voidKeyword";
            break;
        case TokenType::returnKeyword:
            typeStr = "returnKeyword";
            break;
        case TokenType::OpenParenthesis:
            typeStr = "OpenParenthesis";
            break;
        case TokenType::CloseParenthesis:
            typeStr = "CloseParenthesis";
            break;
        case TokenType::OpenBrace:
            typeStr = "OpenBrace";
            break;
        case TokenType::CloseBrace:
            typeStr = "CloseBrace";
            break;
        case TokenType::Semicolon:
            typeStr = "Semicolon";
            break;
        case TokenType::Tilde:
            typeStr = "Tilde";
            break;
        case TokenType::TwoHyphen:
            typeStr = "TwoHyphen";
            break;
        case TokenType::Plus:
            typeStr = "Plus";
            break;
        case TokenType::Minus:
            typeStr = "Minus";
            break;
        case TokenType::Multiply:
            typeStr = "Multiply";
            break;
        case TokenType::Divide:
            typeStr = "Divide";
            break;
        case TokenType::Modulo:
            typeStr = "Modulo";
            break;
        case TokenType::SingleLineComment:
            typeStr = "SingleLineComment";
            break;
        case TokenType::MultiLineComment:
            typeStr = "MultiLineComment";
            break;
        case TokenType::Invalid:
            typeStr = "Invalid";
            break;
        }
        std::cout << typeStr << ": " << token.value << "\n";
    }
}

// Convert the token type to its corresponding string.
std::string tokenTypeToString(TokenType type) {
    std::string typeStr;
    switch (type) {
    case TokenType::Identifier:
        typeStr = "Identifier";
        break;
    case TokenType::Constant:
        typeStr = "Constant";
        break;
    case TokenType::intKeyword:
        typeStr = "intKeyword";
        break;
    case TokenType::voidKeyword:
        typeStr = "voidKeyword";
        break;
    case TokenType::returnKeyword:
        typeStr = "returnKeyword";
        break;
    case TokenType::OpenParenthesis:
        typeStr = "OpenParenthesis";
        break;
    case TokenType::CloseParenthesis:
        typeStr = "CloseParenthesis";
        break;
    case TokenType::OpenBrace:
        typeStr = "OpenBrace";
        break;
    case TokenType::CloseBrace:
        typeStr = "CloseBrace";
        break;
    case TokenType::Semicolon:
        typeStr = "Semicolon";
        break;
    case TokenType::Tilde:
        typeStr = "Tilde";
        break;
    case TokenType::TwoHyphen:
        typeStr = "TwoHyphen";
        break;
    case TokenType::Plus:
        typeStr = "Plus";
        break;
    case TokenType::Minus:
        typeStr = "Minus";
        break;
    case TokenType::Multiply:
        typeStr = "Multiply";
        break;
    case TokenType::Divide:
        typeStr = "Divide";
        break;
    case TokenType::Modulo:
        typeStr = "Modulo";
        break;
    case TokenType::SingleLineComment:
        typeStr = "SingleLineComment";
        break;
    case TokenType::MultiLineComment:
        typeStr = "MultiLineComment";
        break;
    case TokenType::Invalid:
        typeStr = "Invalid";
        break;
    }
    return typeStr;
}
