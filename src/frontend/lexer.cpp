#include "lexer.h"
#include <sstream>

Token matchToken(std::string_view input) {
    // Convert string_view to string for regex operations
    std::string inputStr(input);

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
        return {TokenType::PreprocessorDirective, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, stringLiteral_regex))
        return {TokenType::StringLiteral, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, singleLineComment_regex))
        return {TokenType::SingleLineComment, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, multiLineComment_regex))
        return {TokenType::MultiLineComment, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, LongConstant_regex))
        return {TokenType::LongConstant, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, intConstant_regex))
        return {TokenType::IntConstant, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, intKeyword_regex))
        return {TokenType::intKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, longKeyword_regex))
        return {TokenType::longKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, signedKeyword_regex))
        return {TokenType::signedKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, unsignedKeyword_regex))
        return {TokenType::unsignedKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, voidKeyword_regex))
        return {TokenType::voidKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, returnKeyword_regex))
        return {TokenType::returnKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, ifKeyword_regex))
        return {TokenType::ifKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, elseKeyword_regex))
        return {TokenType::elseKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, doKeyword_regex))
        return {TokenType::doKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, whileKeyword_regex))
        return {TokenType::whileKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, forKeyword_regex))
        return {TokenType::forKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, breakKeyword_regex))
        return {TokenType::breakKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, continueKeyword_regex))
        return {TokenType::continueKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, staticKeyword_regex))
        return {TokenType::staticKeyword, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, externKeyword_regex))
        return {TokenType::externKeyword, tokenMatches.str(0)};
    // Lower down the precedence of token-matching `identifier_regex`
    // to avoid the conflict with the other token matchings (e.g.,
    // `intKeyword_regex`)
    else if (std::regex_search(inputStr, tokenMatches, identifier_regex))
        return {TokenType::Identifier, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, comma_regex))
        return {TokenType::Comma, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, questionMark_regex))
        return {TokenType::QuestionMark, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, colon_regex))
        return {TokenType::Colon, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, openParenthesis_regex))
        return {TokenType::OpenParenthesis, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, closeParenthesis_regex))
        return {TokenType::CloseParenthesis, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, openBrace_regex))
        return {TokenType::OpenBrace, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, closeBrace_regex))
        return {TokenType::CloseBrace, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, semicolon_regex))
        return {TokenType::Semicolon, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, tilde_regex))
        return {TokenType::Tilde, tokenMatches.str(0)};
    // Enforce the precedence of token-matching `twoHyphen_regex` over
    // `minus_regex` to avoid the conflict with the token matching of
    // `minus_regex`.
    else if (std::regex_search(inputStr, tokenMatches, twoHyphen_regex))
        return {TokenType::TwoHyphen, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, plus_regex))
        return {TokenType::Plus, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, minus_regex))
        return {TokenType::Minus, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, multiply_regex))
        return {TokenType::Multiply, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, divide_regex))
        return {TokenType::Divide, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, modulo_regex))
        return {TokenType::Modulo, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, equal_regex))
        return {TokenType::Equal, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, notEqual_regex))
        return {TokenType::NotEqual, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, lessThanOrEqual_regex))
        return {TokenType::LessThanOrEqual, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               greaterThanOrEqual_regex))
        return {TokenType::GreaterThanOrEqual, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, lessThan_regex))
        return {TokenType::LessThan, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, greaterThan_regex))
        return {TokenType::GreaterThan, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalNot_regex))
        return {TokenType::LogicalNot, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalAnd_regex))
        return {TokenType::LogicalAnd, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches, logicalOr_regex))
        return {TokenType::LogicalOr, tokenMatches.str(0)};
    // Lower down the precedence of token-matching `assign_regex` to avoid the
    // conflict with the token matching of `equal_regex`.
    else if (std::regex_search(inputStr, tokenMatches, assign_regex))
        return {TokenType::Assign, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               unsignedLongIntegerConstant_regex))
        return {TokenType::UnsignedLongIntegerConstant, tokenMatches.str(0)};
    else if (std::regex_search(inputStr, tokenMatches,
                               unsignedIntegerConstant_regex))
        return {TokenType::UnsignedIntegerConstant, tokenMatches.str(0)};
    else {
        // If no token matches, throw an error.
        std::stringstream msg;
        msg << "Unknown token: " << inputStr;
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
        Token token = matchToken(remainingInput);

        // If no match is found, raise an error.
        // Print out the remaining input (intentionally) and exit the program.
        if (token.type == TokenType::Invalid) {
            std::stringstream msg;
            msg << "Invalid token found: " << token.value;
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
