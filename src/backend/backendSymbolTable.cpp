#include "backendSymbolTable.h"
#include "../frontend/frontendSymbolTable.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "assemblyGenerator.h"
#include <memory>
#include <stdexcept>
#include <utility>

namespace Assembly {
// Function to convert a frontend symbol table to a backend symbol table.
void convertFrontendToBackendSymbolTable(
    const AST::FrontendSymbolTable &frontendSymbolTable,
    BackendSymbolTable &backendSymbolTable) {
    // Convert each entry from a frontend symbol table to a backend symbol
    // table.
    for (const auto &[identifier, entry] : frontendSymbolTable) {
        auto *astType = entry.first.get();
        auto *identifierAttribute = entry.second.get();

        if (auto *functionAttribute =
                dynamic_cast<AST::FunctionAttribute *>(identifierAttribute)) {
            auto funEntry =
                std::make_unique<FunEntry>(functionAttribute->isDefined());
            backendSymbolTable[identifier] = std::move(funEntry);
        }
        else if (dynamic_cast<AST::StaticAttribute *>(identifierAttribute) !=
                 nullptr) {
            auto assemblyType =
                AssemblyGenerator::convertASTTypeToAssemblyType(astType);
            auto objEntry = std::make_unique<ObjEntry>(
                std::move(assemblyType), true); // true for static storage.
            backendSymbolTable[identifier] = std::move(objEntry);
        }
        else if (dynamic_cast<AST::LocalAttribute *>(identifierAttribute) !=
                 nullptr) {
            auto assemblyType =
                AssemblyGenerator::convertASTTypeToAssemblyType(astType);
            auto objEntry = std::make_unique<ObjEntry>(
                std::move(assemblyType),
                false); // false for non-static storage.
            backendSymbolTable[identifier] = std::move(objEntry);
        }
        else {
            throw std::logic_error(
                "Unsupported identifier attribute type in "
                "convertFrontendToBackendSymbolTable in BackendSymbolTable");
        }
    }
}
} // namespace Assembly
