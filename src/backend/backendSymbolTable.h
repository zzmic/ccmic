#ifndef BACKEND_BACKEND_SYMBOL_TABLE_H
#define BACKEND_BACKEND_SYMBOL_TABLE_H

#include "../frontend/frontendSymbolTable.h"
#include "assembly.h"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <variant>

namespace Assembly {
/**
 * Base class for representing a backend symbol table entry.
 */
class BackendSymbolTableEntry {
  public:
    /**
     * Default constructor for the backend symbol table entry class.
     */
    constexpr BackendSymbolTableEntry() = default;

    /**
     * Default virtual destructor for the backend symbol table entry class.
     */
    virtual ~BackendSymbolTableEntry() = default;

    /**
     * Delete the copy constructor for the backend symbol table entry class.
     */
    constexpr BackendSymbolTableEntry(const BackendSymbolTableEntry &) = delete;

    /**
     * Delete the copy assignment operator for the backend symbol table entry
     * class.
     */
    constexpr BackendSymbolTableEntry &
    operator=(const BackendSymbolTableEntry &) = delete;

    /**
     * Default move constructor for the backend symbol table entry class.
     */
    constexpr BackendSymbolTableEntry(BackendSymbolTableEntry &&) = default;

    /**
     * Default move assignment operator for the backend symbol table entry
     * class.
     */
    constexpr BackendSymbolTableEntry &
    operator=(BackendSymbolTableEntry &&) = default;
};

/**
 * Class for representing an object entry in the backend symbol table.
 */
class ObjEntry : public BackendSymbolTableEntry {
  public:
    /**
     * Constructor for the object entry class.
     *
     * @param assemblyType The assembly type of the object.
     * @param isStatic Whether the object is static.
     */
    explicit ObjEntry(std::unique_ptr<AssemblyType> assemblyType, bool isStatic)
        : assemblyType(std::move(assemblyType)), isStatic(isStatic) {
        if (!this->assemblyType) {
            throw std::logic_error(
                "Creating ObjEntry with null assemblyType in ObjEntry");
        }
    }

    /**
     * Get the assembly type of the object.
     *
     * @return The assembly type of the object.
     */
    [[nodiscard]] const AssemblyType *getAssemblyType() const {
        return assemblyType.get();
    }

    /**
     * Check if the object is static storage.
     *
     * @return True if the object is static storage, false otherwise.
     */
    [[nodiscard]] bool isStaticStorage() const { return isStatic; }

  private:
    /**
     * The assembly type of the object.
     */
    std::unique_ptr<AssemblyType> assemblyType;

    /**
     * Boolean indicating whether the object is static storage.
     */
    bool isStatic = false;
};

/**
 * Class for representing a function entry in the backend symbol table.
 */
class FunEntry : public BackendSymbolTableEntry {
  public:
    /**
     * Constructor for the function entry class.
     *
     * @param defined Boolean indicating whether the function is defined.
     */
    explicit FunEntry(bool defined) : defined(defined) {}

    /**
     * Check if the function is defined.
     *
     * @return True if the function is defined, false otherwise.
     */
    [[nodiscard]] bool isDefined() const { return defined; }

  private:
    /**
     * Boolean indicating whether the function is defined.
     */
    bool defined = false;
};

/**
 * Type alias for the backend symbol table.
 *
 * The key is the identifier (variable or function name), and the value is a
 * unique pointer to the backend symbol table entry.
 */
using BackendSymbolTable =
    std::unordered_map<std::string, std::unique_ptr<BackendSymbolTableEntry>>;

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
