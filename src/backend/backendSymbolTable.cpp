#include "backendSymbolTable.h"
#include "assemblyGenerator.h"

namespace Assembly {
// Function to convert a frontend symbol table to a backend symbol table.
void convertFrontendToBackendSymbolTable(
    const AST::FrontendSymbolTable &frontendSymbolTable,
    BackendSymbolTable &backendSymbolTable) {
    // Convert each entry from a frontend symbol table to a backend symbol
    // table.
    for (const auto &[identifier, entry] : frontendSymbolTable) {
        auto astType = entry.first;
        auto identifierAttribute = entry.second;

        if (auto functionAttribute =
                std::dynamic_pointer_cast<AST::FunctionAttribute>(
                    identifierAttribute)) {
            auto funEntry =
                std::make_shared<FunEntry>(functionAttribute->isDefined());
            backendSymbolTable[identifier] = funEntry;
        }
        else if (auto staticAttribute =
                     std::dynamic_pointer_cast<AST::StaticAttribute>(
                         identifierAttribute)) {
            auto assemblyType =
                AssemblyGenerator::convertASTTypeToAssemblyType(astType);
            auto objEntry = std::make_shared<ObjEntry>(
                assemblyType, true); // true for static storage.
            backendSymbolTable[identifier] = objEntry;
        }
        else if (auto localAttribute =
                     std::dynamic_pointer_cast<AST::LocalAttribute>(
                         identifierAttribute)) {
            auto assemblyType =
                AssemblyGenerator::convertASTTypeToAssemblyType(astType);
            auto objEntry = std::make_shared<ObjEntry>(
                assemblyType,
                false); // false for non-static storage.
            backendSymbolTable[identifier] = objEntry;
        }
        else {
            throw std::logic_error("Unsupported identifier attribute type for "
                                   "backend symbol table conversion");
        }
    }
}
} // namespace Assembly
