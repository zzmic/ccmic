#include "frontendSymbolTable.h"

namespace AST {
// Global frontend symbol table definition.
std::unordered_map<std::string, std::pair<std::shared_ptr<Type>,
                                          std::shared_ptr<IdentifierAttribute>>>
    frontendSymbolTable;
} // namespace AST
