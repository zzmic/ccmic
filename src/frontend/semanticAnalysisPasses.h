#ifndef FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
#define FRONTEND_SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "expression.h"
#include "frontendSymbolTable.h"
#include "program.h"
#include "statement.h"
#include "type.h"
#include <unordered_map>
#include <variant>

namespace AST {
/**
 * Base class for semantic analysis passes.
 */
class SemanticAnalysisPass {
  public:
    /**
     * Default constructor for the semantic analysis pass class.
     */
    constexpr SemanticAnalysisPass() = default;

    /**
     * Virtual destructor for semantic analysis passes.
     */
    virtual ~SemanticAnalysisPass() = default;

    /**
     * Delete the copy constructor for the semantic analysis pass class.
     */
    constexpr SemanticAnalysisPass(const SemanticAnalysisPass &) = delete;

    /**
     * Delete the copy assignment operator for the semantic analysis pass class.
     */
    constexpr SemanticAnalysisPass &
    operator=(const SemanticAnalysisPass &) = delete;

    /**
     * Default move constructor for the semantic analysis pass class.
     */
    constexpr SemanticAnalysisPass(SemanticAnalysisPass &&) = default;

    /**
     * Default move assignment operator for the semantic analysis pass class.
     */
    constexpr SemanticAnalysisPass &
    operator=(SemanticAnalysisPass &&) = default;
};

/**
 * Class representing an entry in the identifier map.
 */
class MapEntry {
  public:
    /**
     * Constructor for the map entry class.
     */
    constexpr MapEntry() = default;

    /**
     * Constructor for the map entry class with parameters.
     *
     * @param newName The new name of the identifier.
     * @param fromCurrentScope Whether the identifier is from the current scope.
     * @param hasLinkage Whether the identifier has linkage.
     */
    explicit MapEntry(std::string newName, bool fromCurrentScope,
                      bool hasLinkage)
        : newName(newName), fromCurrentScope(fromCurrentScope),
          hasLinkage(hasLinkage) {}

    std::string getNewName() { return newName; }

    constexpr bool fromCurrentScopeOrNot() { return fromCurrentScope; }

    constexpr bool hasLinkageOrNot() { return hasLinkage; }

  private:
    /**
     * The new name of the identifier.
     */
    std::string newName;

    /**
     * Whether the identifier is from the current scope.
     */
    bool fromCurrentScope;

    /**
     * Whether the identifier has linkage.
     */
    bool hasLinkage;
};

/**
 * Class for performing identifier resolution in the AST.
 */
class IdentifierResolutionPass : public SemanticAnalysisPass {
  public:
    /**
     * Resolve identifiers in the given program.
     *
     * @param program The program to resolve.
     * @return The number of resolved identifiers.
     */
    int resolveProgram(Program &program);

  private:
    /**
     * Counter for generating unique variable names.
     */
    int variableResolutionCounter = 0;

    /**
     * Generate a unique variable name based on the given identifier.
     *
     * @param identifier The original identifier.
     * @return A unique variable name.
     */
    std::string generateUniqueVariableName(const std::string &identifier);

    /**
     * Copy the identifier map, setting `fromCurrentScope` to false for all
     * entries.
     *
     * @param identifierMap The original identifier map.
     * @return The copied identifier map.
     */
    std::unordered_map<std::string, MapEntry>
    copyIdentifierMap(std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a file-scope variable declaration.
     *
     * @param declaration The variable declaration to resolve.
     * @param identifierMap The current identifier map.
     */
    void resolveFileScopeVariableDeclaration(
        VariableDeclaration *declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a local variable declaration.
     *
     * @param declaration The variable declaration to resolve.
     * @param identifierMap The current identifier map.
     */
    void resolveLocalVariableDeclaration(
        VariableDeclaration *declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a statement.
     *
     * @param statement The statement to resolve.
     * @param identifierMap The current identifier map.
     */
    void
    resolveStatement(Statement *statement,
                     std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve an expression.
     *
     * @param expression The expression to resolve.
     * @param identifierMap The current identifier map.
     */
    void
    resolveExpression(Expression *expression,
                      std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a block.
     *
     * @param block The block to resolve.
     * @param identifierMap The current identifier map.
     */
    void resolveBlock(Block *block,
                      std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a for-init.
     *
     * @param forInit The for-init to resolve.
     * @param identifierMap The current identifier map.
     */
    void
    resolveForInit(ForInit *forInit,
                   std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a function declaration.
     *
     * @param declaration The function declaration to resolve.
     * @param identifierMap The current identifier map.
     */
    void resolveFunctionDeclaration(
        FunctionDeclaration *declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a parameter identifier.
     *
     * @param parameter The parameter identifier to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved parameter identifier.
     */
    std::string
    resolveParameter(std::string parameter,
                     std::unordered_map<std::string, MapEntry> &identifierMap);
};

/**
 * Base class for static initializers in the AST.
 */
class StaticInit {
  public:
    /**
     * Default constructor for the static initializer class.
     */
    constexpr StaticInit() = default;

    /**
     * Default virtual destructor for static initializers.
     */
    virtual ~StaticInit() = default;

    /**
     * Delete the copy constructor for the static initializer class.
     */
    constexpr StaticInit(const StaticInit &) = delete;

    /**
     * Delete the copy assignment operator for the static initializer class.
     */
    constexpr StaticInit &operator=(const StaticInit &) = delete;

    /**
     * Default move constructor for the static initializer class.
     */
    constexpr StaticInit(StaticInit &&) = default;

    /**
     * Default move assignment operator for the static initializer class.
     */
    constexpr StaticInit &operator=(StaticInit &&) = default;

    /**
     * Pure virtual method to get the value of the static initializer.
     */
    virtual std::variant<int, long, unsigned int, unsigned long>
    getValue() const = 0;
};

/**
 * Class representing an integer static initializer.
 */
class IntInit : public StaticInit {
  public:
    /**
     * Constructor for the integer static initializer class.
     *
     * @param value The integer value of the static initializer.
     */
    constexpr IntInit(int value) : value(value) {}

    std::variant<int, long, unsigned int, unsigned long>
    getValue() const override {
        return value;
    }

  private:
    /**
     * The integer value of the static initializer.
     */
    int value;
};

class LongInit : public StaticInit {
  public:
    /**
     * Constructor for the long static initializer class.
     *
     * @param value The long value of the static initializer.
     */
    constexpr LongInit(long value) : value(value) {}

    std::variant<int, long, unsigned int, unsigned long>
    getValue() const override {
        return value;
    }

  private:
    /**
     * The long value of the static initializer.
     */
    long value;
};

class UIntInit : public StaticInit {
  public:
    /**
     * Constructor for the unsigned integer static initializer class.
     *
     * @param value The unsigned integer value of the static initializer.
     */
    constexpr UIntInit(unsigned int value) : value(value) {}

    std::variant<int, long, unsigned int, unsigned long>
    getValue() const override {
        return value;
    }

  private:
    /**
     * The unsigned integer value of the static initializer.
     */
    unsigned int value;
};

class ULongInit : public StaticInit {
  public:
    /**
     * Constructor for the unsigned long static initializer class.
     *
     * @param value The unsigned long value of the static initializer.
     */
    constexpr ULongInit(unsigned long value) : value(value) {}

    std::variant<int, long, unsigned int, unsigned long>
    getValue() const override {
        return value;
    }

  private:
    /**
     * The unsigned long value of the static initializer.
     */
    unsigned long value;
};

/**
 * Base class for initial values in the AST.
 */
class InitialValue {
  public:
    /**
     * Default constructor for the initial value class.
     */
    constexpr InitialValue() = default;

    /**
     * Default virtual destructor for initial values.
     */
    virtual ~InitialValue() = default;

    /**
     * Delete the copy constructor for the initial value class.
     */
    constexpr InitialValue(const InitialValue &) = delete;

    /**
     * Delete the copy assignment operator for the initial value class.
     */
    constexpr InitialValue &operator=(const InitialValue &) = delete;

    /**
     * Default move constructor for the initial value class.
     */
    constexpr InitialValue(InitialValue &&) = default;

    /**
     * Default move assignment operator for the initial value class.
     */
    constexpr InitialValue &operator=(InitialValue &&) = default;
};

/**
 * Class representing a tentative initial.
 */
class Tentative : public InitialValue {};

/**
 * Class representing an initial with a static initializer.
 */
class Initial : public InitialValue {
  public:
    /**
     * Constructor for the initial class with an `int` static initializer.
     *
     * @param value The `int` value of the static initializer.
     */
    Initial(int value) : staticInit(std::make_unique<IntInit>(value)) {}

    /**
     * Constructor for the initial class with a `long` static initializer.
     *
     * @param value The `long` value of the static initializer.
     */
    Initial(long value) : staticInit(std::make_unique<LongInit>(value)) {}

    /**
     * Constructor for the initial class with an `unsigned int` static
     * initializer.
     *
     * @param value The `unsigned int` value of the static initializer.
     */
    Initial(unsigned int value)
        : staticInit(std::make_unique<UIntInit>(value)) {}

    /**
     * Constructor for the initial class with an `unsigned long` static
     * initializer.
     *
     * @param value The `unsigned long` value of the static initializer.
     */
    Initial(unsigned long value)
        : staticInit(std::make_unique<ULongInit>(value)) {}

    /**
     * Constructor for the initial class with a static initializer parameter.
     *
     * @param staticInit The static initializer.
     */
    explicit Initial(std::unique_ptr<StaticInit> staticInit)
        : staticInit(std::move(staticInit)) {}

    StaticInit *getStaticInit() const { return staticInit.get(); }

  private:
    /**
     * The static initializer.
     */
    std::unique_ptr<StaticInit> staticInit;
};

/**
 * Class representing no initializer.
 */
class NoInitializer : public InitialValue {};

/**
 * Base class for identifier attributes in the AST.
 */
class IdentifierAttribute {
  public:
    /**
     * Default constructor for the identifier attribute class.
     */
    constexpr IdentifierAttribute() = default;

    /**
     * Default virtual destructor for identifier attributes.
     */
    virtual ~IdentifierAttribute() = default;

    /**
     * Delete the copy constructor for the identifier attribute class.
     */
    constexpr IdentifierAttribute(const IdentifierAttribute &) = delete;

    /**
     * Delete the copy assignment operator for the identifier attribute class.
     */
    constexpr IdentifierAttribute &
    operator=(const IdentifierAttribute &) = delete;

    /**
     * Default move constructor for the identifier attribute class.
     */
    constexpr IdentifierAttribute(IdentifierAttribute &&) = default;

    /**
     * Default move assignment operator for the identifier attribute class.
     */
    constexpr IdentifierAttribute &operator=(IdentifierAttribute &&) = default;
};

/**
 * Class representing function attributes.
 */
class FunctionAttribute : public IdentifierAttribute {
  public:
    /**
     * Constructor for the function attribute class.
     *
     * @param defined Whether the function is defined.
     * @param global Whether the function is global.
     */
    constexpr FunctionAttribute(bool defined, bool global)
        : defined(defined), global(global) {}

    constexpr bool isDefined() { return defined; }

    constexpr bool isGlobal() { return global; }

  private:
    /**
     * Whether the function is defined.
     */
    bool defined;

    /**
     * Whether the function is global.
     */
    bool global;
};

/**
 * Class representing static variable attributes.
 */
class StaticAttribute : public IdentifierAttribute {
  public:
    /**
     * Constructor for the static attribute class.
     *
     * @param initialValue The initial value of the static variable.
     * @param global Whether the static variable is global.
     */
    StaticAttribute(std::unique_ptr<InitialValue> initialValue, bool global)
        : initialValue(std::move(initialValue)), global(global) {}

    InitialValue *getInitialValue() { return initialValue.get(); }

    constexpr bool isGlobal() { return global; }

  private:
    /**
     * The initial value of the static variable.
     */
    std::unique_ptr<InitialValue> initialValue;

    /**
     * Whether the static variable is global.
     */
    bool global;
};

/**
 * Class representing local variable attributes.
 */
class LocalAttribute : public IdentifierAttribute {};

/**
 * Class representing the type checking pass in semantic analysis.
 */
class TypeCheckingPass : public SemanticAnalysisPass {
  public:
    /**
     * Constructor for the type checking pass class.
     *
     * @param frontendSymbolTable The frontend symbol table.
     */
    explicit TypeCheckingPass(FrontendSymbolTable &frontendSymbolTable);

    /**
     * Type check the given program.
     *
     * @param program The program to type check.
     */
    void typeCheckProgram(Program &program);

  private:
    /**
     * The frontend symbol table.
     */
    FrontendSymbolTable &frontendSymbolTable;

    /**
     * Convert a static constant expression to a static initializer.
     *
     * @param varType The type of the variable.
     * @param constantExpr The constant expression to
     */
    std::unique_ptr<StaticInit>
    convertStaticConstantToStaticInit(const Type *varType,
                                      const ConstantExpression *constantExpr);

    /**
     * Get the common type between two types.
     *
     * @param type1 The first type.
     * @param type2 The second type.
     */
    std::unique_ptr<Type> getCommonType(const Type *type1, const Type *type2);

    /**
     * Convert an expression to a target type.
     *
     * @param expression The expression to convert.
     * @param targetType The target type.
     */
    std::unique_ptr<Expression> convertTo(const Expression *expression,
                                          const Type *targetType);

    /**
     * Type check a function declaration.
     *
     * @param declaration The function declaration to type check.
     */
    void typeCheckFunctionDeclaration(FunctionDeclaration *declaration);

    /**
     * Type check a file-scope variable declaration.
     *
     * @param declaration The variable declaration to type check.
     */
    void
    typeCheckFileScopeVariableDeclaration(VariableDeclaration *declaration);

    /**
     * Type check a local variable declaration.
     *
     * @param declaration The variable declaration to type check.
     */
    void typeCheckLocalVariableDeclaration(VariableDeclaration *declaration);

    /**
     * Type check an expression.
     *
     * @param expression The expression to type check.
     */
    void typeCheckExpression(Expression *expression);

    /**
     * Type check a block.
     *
     * @param block The block to type check.
     * @param enclosingFunctionIdentifier The identifier of the enclosing
     * function.
     */
    void typeCheckBlock(Block *block, std::string enclosingFunctionIdentifier);

    /**
     * Type check a statement.
     *
     * @param statement The statement to type check.
     * @param enclosingFunctionIdentifier The identifier of the enclosing
     * function.
     */
    void typeCheckStatement(Statement *statement,
                            std::string enclosingFunctionIdentifier);

    /**
     * Type check a for-init.
     *
     * @param forInit The for-init to type check.
     */
    void typeCheckForInit(ForInit *forInit);
};

/**
 * Class representing the loop labeling pass in semantic analysis.
 */
class LoopLabelingPass : public SemanticAnalysisPass {
  public:
    /**
     * Label loops in the given program.
     *
     * @param program The program to label loops in.
     */
    void labelLoops(Program &program);

  private:
    /**
     * Counter for generating unique loop labels.
     */
    int loopLabelingCounter = 0;

    /**
     * Generate a unique loop label.
     *
     * @return A unique loop label.
     */
    std::string generateLoopLabel();

    /**
     * Annotate a statement with a label.
     *
     * @param statement The statement to annotate.
     * @param label The label to annotate with.
     */
    void annotateStatement(Statement *statement, std::string_view label);

    /**
     * Label a statement with a label.
     *
     * @param statement The statement to label.
     * @param label The label to use.
     */
    void labelStatement(Statement *statement, std::string_view label);

    /**
     * Label a block with a label.
     *
     * @param block The block to label.
     * @param label The label to use.
     */
    void labelBlock(Block *block, std::string_view label);
};
} // namespace AST

#endif // FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
