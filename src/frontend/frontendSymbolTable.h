#ifndef FRONTEND_SYMBOL_TABLE_H
#define FRONTEND_SYMBOL_TABLE_H

#include "semanticAnalysisPasses.h"
#include "type.h"
#include <memory>
#include <unordered_map>

namespace AST {
// Global frontend symbol table declaration.
extern std::unordered_map<
    std::string,
    std::pair<std::shared_ptr<Type>, std::shared_ptr<IdentifierAttribute>>>
    frontendSymbolTable;
} // namespace AST

#endif // FRONTEND_SYMBOL_TABLE_H
