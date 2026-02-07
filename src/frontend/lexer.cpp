#include "lexer.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

Token matchToken(std::string_view input) {
    // Convert string_view to string for regex operations
    const std::string inputStr(input);

    // Instantiate the `match_results` class template for matches on string
    // objects.
    std::smatch tokenMatches;

    // Match the input string against the regular expressions for the different
    // token types and return the token struct, containing the token type and
    // the token value (i,e., the complete match, which is the entire portion of
    // the string that matched the regex). Raise up the precedence of
    // token-matching `singleLineComment_regex` and `multiLineComment_regex` to
    // resolve the conflict with the other token matchings (e.g.,
    // `multiply_regex`).
    if (std::regex_search(inputStr, tokenMatches, preprocessorDirective_regex))
        return {.type = TokenType::PreprocessorDirective,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, stringLiteral_regex))
        return {.type = TokenType::StringLiteral, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, singleLineComment_regex))
        return {.type = TokenType::SingleLineComment,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, multiLineComment_regex))
        return {.type = TokenType::MultiLineComment,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, LongConstant_regex))
        return {.type = TokenType::LongConstant, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, intConstant_regex))
        return {.type = TokenType::IntConstant, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, intKeyword_regex))
        return {.type = TokenType::intKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, longKeyword_regex))
        return {.type = TokenType::longKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, signedKeyword_regex))
        return {.type = TokenType::signedKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, unsignedKeyword_regex))
        return {.type = TokenType::unsignedKeyword,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, voidKeyword_regex))
        return {.type = TokenType::voidKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, returnKeyword_regex))
        return {.type = TokenType::returnKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, ifKeyword_regex))
        return {.type = TokenType::ifKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, elseKeyword_regex))
        return {.type = TokenType::elseKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, doKeyword_regex))
        return {.type = TokenType::doKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, whileKeyword_regex))
        return {.type = TokenType::whileKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, forKeyword_regex))
        return {.type = TokenType::forKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, breakKeyword_regex))
        return {.type = TokenType::breakKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, continueKeyword_regex))
        return {.type = TokenType::continueKeyword,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, staticKeyword_regex))
        return {.type = TokenType::staticKeyword, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, externKeyword_regex))
        return {.type = TokenType::externKeyword, .value = tokenMatches.str(0)};
    // Lower down the precedence of token-matching `identifier_regex`
    // to avoid the conflict with the other token matchings (e.g.,
    // `intKeyword_regex`)
    else if (std::regex_search(inputStr, tokenMatches, identifier_regex))
        return {.type = TokenType::Identifier, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, comma_regex))
        return {.type = TokenType::Comma, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, questionMark_regex))
        return {.type = TokenType::QuestionMark, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, colon_regex))
        return {.type = TokenType::Colon, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, openParenthesis_regex))
        return {.type = TokenType::OpenParenthesis,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, closeParenthesis_regex))
        return {.type = TokenType::CloseParenthesis,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, openBrace_regex))
        return {.type = TokenType::OpenBrace, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, closeBrace_regex))
        return {.type = TokenType::CloseBrace, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, semicolon_regex))
        return {.type = TokenType::Semicolon, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, tilde_regex))
        return {.type = TokenType::Tilde, .value = tokenMatches.str(0)};
    // Enforce the precedence of token-matching `twoHyphen_regex` over
    // `minus_regex` to avoid the conflict with the token matching of
    // `minus_regex`.
    else if (std::regex_search(inputStr, tokenMatches, twoHyphen_regex))
        return {.type = TokenType::TwoHyphen, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, plus_regex))
        return {.type = TokenType::Plus, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, minus_regex))
        return {.type = TokenType::Minus, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, multiply_regex))
        return {.type = TokenType::Multiply, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, divide_regex))
        return {.type = TokenType::Divide, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, modulo_regex))
        return {.type = TokenType::Modulo, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, equal_regex))
        return {.type = TokenType::Equal, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, notEqual_regex))
        return {.type = TokenType::NotEqual, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, lessThanOrEqual_regex))
        return {.type = TokenType::LessThanOrEqual,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               greaterThanOrEqual_regex))
        return {.type = TokenType::GreaterThanOrEqual,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, lessThan_regex))
        return {.type = TokenType::LessThan, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, greaterThan_regex))
        return {.type = TokenType::GreaterThan, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalNot_regex))
        return {.type = TokenType::LogicalNot, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalAnd_regex))
        return {.type = TokenType::LogicalAnd, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalOr_regex))
        return {.type = TokenType::LogicalOr, .value = tokenMatches.str(0)};
    // Lower down the precedence of token-matching `assign_regex` to avoid the
    // conflict with the token matching of `equal_regex`.
    else if (std::regex_search(inputStr, tokenMatches, assign_regex))
        return {.type = TokenType::Assign, .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               unsignedLongIntegerConstant_regex))
        return {.type = TokenType::UnsignedLongIntegerConstant,
                .value = tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               unsignedIntegerConstant_regex))
        return {.type = TokenType::UnsignedIntegerConstant,
                .value = tokenMatches.str(0)};
    else {
        std::stringstream msg;
        msg << "Invalid token found in matchToken in Lexer: " << inputStr;
        throw std::invalid_argument(msg.str());
    }
}

std::vector<Token> lexer(std::string_view input) {
    std::vector<Token> tokens;
    std::string remainingInput(input);

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
        const Token token = matchToken(remainingInput);

        // If no match is found, raise an error.
        // Print out the remaining input (intentionally) and exit the program.
        if (token.type == TokenType::Invalid) {
            std::stringstream msg;
            msg << "Invalid token found in lexer in Lexer: " << token.value;
            throw std::invalid_argument(msg.str());
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
                const Token nextToken = matchToken(remainingInput);
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
        case TokenType::signedKeyword:
            typeStr = "signedKeyword";
            break;
        case TokenType::unsignedKeyword:
            typeStr = "unsignedKeyword";
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
        case TokenType::UnsignedIntegerConstant:
            typeStr = "UnsignedIntegerConstant";
            break;
        case TokenType::UnsignedLongIntegerConstant:
            typeStr = "UnsignedLongIntegerConstant";
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
    case TokenType::signedKeyword:
        typeStr = "signedKeyword";
        break;
    case TokenType::unsignedKeyword:
        typeStr = "unsignedKeyword";
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
    case TokenType::UnsignedIntegerConstant:
        typeStr = "UnsignedIntegerConstant";
        break;
    case TokenType::UnsignedLongIntegerConstant:
        typeStr = "UnsignedLongIntegerConstant";
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
