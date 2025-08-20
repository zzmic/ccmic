#include "frontendSymbolTable.h"
#include <memory>

namespace AST {
// Global frontend symbol table definition.
std::unordered_map<std::string, std::pair<std::unique_ptr<Type>,
                                          std::unique_ptr<IdentifierAttribute>>>
    frontendSymbolTable;
} // namespace AST
