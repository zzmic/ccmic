#ifndef BACKEND_BACKEND_SYMBOL_TABLE_H
#define BACKEND_BACKEND_SYMBOL_TABLE_H

#include "../frontend/semanticAnalysisPasses.h"
#include "assembly.h"
#include <memory>
#include <unordered_map>
#include <variant>

namespace Assembly {
class BackendSymbolTableEntry {
  public:
    virtual ~BackendSymbolTableEntry() = default;
};

class ObjEntry : public BackendSymbolTableEntry {
  public:
    explicit ObjEntry(std::shared_ptr<AssemblyType> assemblyType, bool isStatic)
        : assemblyType(assemblyType), isStatic(isStatic) {}
    [[nodiscard]] std::shared_ptr<AssemblyType> getAssemblyType() const {
        return assemblyType;
    }
    [[nodiscard]] bool isStaticStorage() const { return isStatic; }

  private:
    std::shared_ptr<AssemblyType> assemblyType;
    bool isStatic;
};

class FunEntry : public BackendSymbolTableEntry {
  public:
    explicit FunEntry(bool defined) : defined(defined) {}
    [[nodiscard]] bool isDefined() const { return defined; }

  private:
    bool defined;
};

// Global backend symbol table declaration.
extern std::unordered_map<std::string, std::shared_ptr<BackendSymbolTableEntry>>
    backendSymbolTable;

// Function to convert a frontend symbol table to a backend symbol table.
void convertFrontendToBackendSymbolTable(
    const std::unordered_map<
        std::string, std::pair<std::shared_ptr<AST::Type>,
                               std::shared_ptr<AST::IdentifierAttribute>>>
        &frontendSymbolTable);
} // namespace Assembly

#endif // BACKEND_BACKEND_SYMBOL_TABLE_H
