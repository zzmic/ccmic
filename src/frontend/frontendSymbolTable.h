#ifndef FRONTEND_SYMBOL_TABLE_H
#define FRONTEND_SYMBOL_TABLE_H

#include "semanticAnalysisPasses.h"
#include "type.h"
#include <memory>
#include <unordered_map>

namespace AST {
/**
 * The symbol table shared across the frontend.
 *
 * It maps identifiers (variable and function names) to their corresponding
 * types and (identifier) attributes.
 */
extern std::unordered_map<
    std::string,
    std::pair<std::shared_ptr<Type>, std::shared_ptr<IdentifierAttribute>>>
    frontendSymbolTable;
} // namespace AST

#endif // FRONTEND_SYMBOL_TABLE_H
