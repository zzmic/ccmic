#include "lexer.h"
#include <sstream>

// Function to match an input token against the regular expressions for the
// different token types and return the token struct, containing the token type
// and the token value (in string).
Token matchToken(const std::string &input) {
    // Instantiate the `match_results` class template for matches on string
    // objects.
    std::smatch tokenMatch;

    // Match the input string against the regular expressions for the different
    // token types and return the token struct, containing the token type and
    // the token value (in string).
    // Raise up the precedence of token-matching `singleLineComment_regex` and
    // `multiLineComment_regex` to resolve the conflict with the other token
    // matchings (e.g., `multiply_regex`).
    if (std::regex_search(input, tokenMatch, preprocessorDirective_regex))
        return {TokenType::PreprocessorDirective, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, stringLiteral_regex))
        return {TokenType::StringLiteral, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, singleLineComment_regex))
        return {TokenType::SingleLineComment, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, multiLineComment_regex))
        return {TokenType::MultiLineComment, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, LongConstant_regex))
        return {TokenType::LongConstant, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, intConstant_regex))
        return {TokenType::IntConstant, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, intKeyword_regex))
        return {TokenType::intKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, longKeyword_regex))
        return {TokenType::longKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, voidKeyword_regex))
        return {TokenType::voidKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, returnKeyword_regex))
        return {TokenType::returnKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, ifKeyword_regex))
        return {TokenType::ifKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, elseKeyword_regex))
        return {TokenType::elseKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, doKeyword_regex))
        return {TokenType::doKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, whileKeyword_regex))
        return {TokenType::whileKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, forKeyword_regex))
        return {TokenType::forKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, breakKeyword_regex))
        return {TokenType::breakKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, continueKeyword_regex))
        return {TokenType::continueKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, staticKeyword_regex))
        return {TokenType::staticKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, externKeyword_regex))
        return {TokenType::externKeyword, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, comma_regex))
        return {TokenType::Comma, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, questionMark_regex))
        return {TokenType::QuestionMark, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, colon_regex))
        return {TokenType::Colon, tokenMatch.str()};
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
    // `minus_regex` to avoid the conflict with the token matching of
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
    else if (std::regex_search(input, tokenMatch, equal_regex))
        return {TokenType::Equal, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, notEqual_regex))
        return {TokenType::NotEqual, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, lessThanOrEqual_regex))
        return {TokenType::LessThanOrEqual, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, greaterThanOrEqual_regex))
        return {TokenType::GreaterThanOrEqual, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, lessThan_regex))
        return {TokenType::LessThan, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, greaterThan_regex))
        return {TokenType::GreaterThan, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, logicalNot_regex))
        return {TokenType::LogicalNot, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, logicalAnd_regex))
        return {TokenType::LogicalAnd, tokenMatch.str()};
    else if (std::regex_search(input, tokenMatch, logicalOr_regex))
        return {TokenType::LogicalOr, tokenMatch.str()};
    // Lower down the precedence of token-matching `assign_regex` to avoid the
    // conflict with the token matching of `equal_regex`.
    else if (std::regex_search(input, tokenMatch, assign_regex))
        return {TokenType::Assign, tokenMatch.str()};
    // Lower down the precedence of token-matching `identifier_regex`
    // to avoid the conflict with the other token matchings (e.g.,
    // `intKeyword_regex`)
    else if (std::regex_search(input, tokenMatch, identifier_regex))
        return {TokenType::Identifier, tokenMatch.str()};
    // If no match is found, return an "invalid" token.
    else
        return {TokenType::Invalid, input};
}

// Function to tokenize the input string and return a vector of tokens.
std::vector<Token> lexer(const std::string &input) {
    std::vector<Token> tokens;
    std::string remainingInput = input;

    // Process the input string until it is empty.
    while (!remainingInput.empty()) {
        // If the input starts with some whitespace, trim the whitespace from
        // the start of the input.
        remainingInput =
            std::regex_replace(remainingInput, std::regex("^\\s+"), "");
        if (remainingInput.empty())
            break;

        // Find the longest match at the start of the input for any regex
        // specified in Table 1-1 (page 9).
        Token token = matchToken(remainingInput);

        // If no match is found, raise an error.
        // Print out the remaining input (intentionally) and exit the program.
        if (token.type == TokenType::Invalid) {
            std::stringstream msg;
            msg << "Invalid token found: " << token.value;
            throw std::runtime_error(msg.str());
        }

        // Skip preprocessor directives and their corresponding tokens (e.g.,
        // string literals).
        if (token.type == TokenType::PreprocessorDirective) {
            remainingInput = remainingInput.substr(token.value.size());
            // Skip any additional tokens, such as string literals, that might
            // follow a directive.
            while (!remainingInput.empty()) {
                remainingInput =
                    std::regex_replace(remainingInput, std::regex("^\\s+"), "");
                Token nextToken = matchToken(remainingInput);
                // Stop skipping if the next token is not part of the directive
                if (nextToken.type != TokenType::StringLiteral &&
                    nextToken.type != TokenType::Identifier) {
                    break;
                }
                remainingInput = remainingInput.substr(nextToken.value.size());
            }
            continue;
        }

        // Skip the token (matching substring) if the token is of type
        // `SingleLineComment` or `MultiLineComment`.
        if (token.type == TokenType::SingleLineComment ||
            token.type == TokenType::MultiLineComment) {
            remainingInput = remainingInput.substr(token.value.size());
            continue;
        }

        // Add the token (matching substring) to the list of tokens except for
        // tokens of types `SingleLineComment` and `MultiLineComment`.
        tokens.emplace_back(token);

        // Remove the token (matching substring) from the start of the input.
        remainingInput = remainingInput.substr(token.value.size());
    }

    // Return the vector of tokens.
    return tokens;
}

// Function to pretty-print the tokens.
void printTokens(const std::vector<Token> &tokens) {
    // For each token, print the token type (converted from each token type to
    // its corresponding string) and the token value.
    for (const auto &token : tokens) {
        std::string typeStr;
        switch (token.type) {
        case TokenType::Identifier:
            typeStr = "Identifier";
            break;
        case TokenType::LongConstant:
            typeStr = "LongConstant";
            break;
        case TokenType::IntConstant:
            typeStr = "IntConstant";
            break;
        case TokenType::intKeyword:
            typeStr = "intKeyword";
            break;
        case TokenType::longKeyword:
            typeStr = "longKeyword";
            break;
        case TokenType::voidKeyword:
            typeStr = "voidKeyword";
            break;
        case TokenType::returnKeyword:
            typeStr = "returnKeyword";
            break;
        case TokenType::ifKeyword:
            typeStr = "ifKeyword";
            break;
        case TokenType::elseKeyword:
            typeStr = "elseKeyword";
            break;
        case TokenType::doKeyword:
            typeStr = "doKeyword";
            break;
        case TokenType::whileKeyword:
            typeStr = "whileKeyword";
            break;
        case TokenType::forKeyword:
            typeStr = "forKeyword";
            break;
        case TokenType::breakKeyword:
            typeStr = "breakKeyword";
            break;
        case TokenType::continueKeyword:
            typeStr = "continueKeyword";
            break;
        case TokenType::staticKeyword:
            typeStr = "staticKeyword";
            break;
        case TokenType::externKeyword:
            typeStr = "externKeyword";
            break;
        case TokenType::Comma:
            typeStr = "Comma";
            break;
        case TokenType::QuestionMark:
            typeStr = "QuestionMark";
            break;
        case TokenType::Colon:
            typeStr = "Colon";
            break;
        case TokenType::Assign:
            typeStr = "Assign";
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
        case TokenType::LogicalNot:
            typeStr = "LogicalNot";
            break;
        case TokenType::LogicalAnd:
            typeStr = "LogicalAnd";
            break;
        case TokenType::LogicalOr:
            typeStr = "LogicalOr";
            break;
        case TokenType::Equal:
            typeStr = "Equal";
            break;
        case TokenType::NotEqual:
            typeStr = "NotEqual";
            break;
        case TokenType::LessThanOrEqual:
            typeStr = "LessThanOrEqual";
            break;
        case TokenType::GreaterThanOrEqual:
            typeStr = "GreaterThanOrEqual";
            break;
        case TokenType::LessThan:
            typeStr = "LessThan";
            break;
        case TokenType::GreaterThan:
            typeStr = "GreaterThan";
            break;
        case TokenType::SingleLineComment:
            typeStr = "SingleLineComment";
            break;
        case TokenType::MultiLineComment:
            typeStr = "MultiLineComment";
            break;
        case TokenType::PreprocessorDirective:
            typeStr = "PreprocessorDirective";
            break;
        case TokenType::StringLiteral:
            typeStr = "StringLiteral";
            break;
        case TokenType::Invalid:
            typeStr = "Invalid";
            break;
        }
        std::cout << typeStr << ": " << token.value << "\n";
    }
}

// Function to convert each token type to its corresponding string
// representation.
std::string tokenTypeToString(TokenType type) {
    std::string typeStr;
    switch (type) {
    case TokenType::Identifier:
        typeStr = "Identifier";
        break;
    case TokenType::LongConstant:
        typeStr = "LongConstant";
        break;
    case TokenType::IntConstant:
        typeStr = "IntConstant";
        break;
    case TokenType::intKeyword:
        typeStr = "intKeyword";
        break;
    case TokenType::longKeyword:
        typeStr = "longKeyword";
        break;
    case TokenType::voidKeyword:
        typeStr = "voidKeyword";
        break;
    case TokenType::returnKeyword:
        typeStr = "returnKeyword";
        break;
    case TokenType::ifKeyword:
        typeStr = "ifKeyword";
        break;
    case TokenType::elseKeyword:
        typeStr = "elseKeyword";
        break;
    case TokenType::doKeyword:
        typeStr = "doKeyword";
        break;
    case TokenType::whileKeyword:
        typeStr = "whileKeyword";
        break;
    case TokenType::forKeyword:
        typeStr = "forKeyword";
        break;
    case TokenType::breakKeyword:
        typeStr = "breakKeyword";
        break;
    case TokenType::continueKeyword:
        typeStr = "continueKeyword";
        break;
    case TokenType::staticKeyword:
        typeStr = "staticKeyword";
        break;
    case TokenType::externKeyword:
        typeStr = "externKeyword";
        break;
    case TokenType::Comma:
        typeStr = "Comma";
        break;
    case TokenType::QuestionMark:
        typeStr = "QuestionMark";
        break;
    case TokenType::Colon:
        typeStr = "Colon";
        break;
    case TokenType::Assign:
        typeStr = "Assign";
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
    case TokenType::LogicalNot:
        typeStr = "LogicalNot";
        break;
    case TokenType::LogicalAnd:
        typeStr = "LogicalAnd";
        break;
    case TokenType::LogicalOr:
        typeStr = "LogicalOr";
        break;
    case TokenType::Equal:
        typeStr = "Equal";
        break;
    case TokenType::NotEqual:
        typeStr = "NotEqual";
        break;
    case TokenType::LessThanOrEqual:
        typeStr = "LessThanOrEqual";
        break;
    case TokenType::GreaterThanOrEqual:
        typeStr = "GreaterThanOrEqual";
        break;
    case TokenType::LessThan:
        typeStr = "LessThan";
        break;
    case TokenType::GreaterThan:
        typeStr = "GreaterThan";
        break;
    case TokenType::SingleLineComment:
        typeStr = "SingleLineComment";
        break;
    case TokenType::MultiLineComment:
        typeStr = "MultiLineComment";
        break;
    case TokenType::PreprocessorDirective:
        typeStr = "PreprocessorDirective";
        break;
    case TokenType::StringLiteral:
        typeStr = "StringLiteral";
        break;
    case TokenType::Invalid:
        typeStr = "Invalid";
        break;
    }
    return typeStr;
}
