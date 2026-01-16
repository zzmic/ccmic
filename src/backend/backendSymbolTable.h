#ifndef BACKEND_BACKEND_SYMBOL_TABLE_H
#define BACKEND_BACKEND_SYMBOL_TABLE_H

#include "../frontend/frontendSymbolTable.h"
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

/**
 * Type alias for the backend symbol table.
 *
 * The key is the identifier (variable or function name), and the value is a
 * shared pointer to the backend symbol table entry.
 */
using BackendSymbolTable =
    std::unordered_map<std::string, std::shared_ptr<BackendSymbolTableEntry>>;

/**
 * Convert a frontend symbol table to a backend symbol table.
 *
 * @param frontendSymbolTable The frontend symbol table to convert.
 * @param backendSymbolTable The backend symbol table to populate.
 */
void convertFrontendToBackendSymbolTable(
    const AST::FrontendSymbolTable &frontendSymbolTable,
    BackendSymbolTable &backendSymbolTable);
} // namespace Assembly

#endif // BACKEND_BACKEND_SYMBOL_TABLE_H
