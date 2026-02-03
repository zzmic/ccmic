#ifndef FRONTEND_SYMBOL_TABLE_H
#define FRONTEND_SYMBOL_TABLE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace AST {
class Type;
class IdentifierAttribute;

/**
 * Type alias for the frontend symbol table.
 *
 * The key is the identifier (variable or function name), and the value is a
 * pair consisting of (a unique pointer to) the type and (a unique pointer to)
 * the identifier attribute.
 */
using FrontendSymbolTable = std::unordered_map<
    std::string,
    std::pair<std::unique_ptr<Type>, std::unique_ptr<IdentifierAttribute>>>;
} // namespace AST

#endif // FRONTEND_SYMBOL_TABLE_H
