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
     * Virtual destructor for semantic analysis passes.
     */
    virtual ~SemanticAnalysisPass() = default;
};

/**
 * Class representing an entry in the identifier map.
 */
class MapEntry {
  public:
    /**
     * Constructor of the map entry class.
     *
     */
    explicit MapEntry() = default;

    /**
     * Constructor of the map entry class with parameters.
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
    int resolveProgram(std::shared_ptr<Program> program);

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
     * @return The resolved variable declaration.
     */
    std::shared_ptr<Declaration> resolveFileScopeVariableDeclaration(
        std::shared_ptr<Declaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a local variable declaration.
     *
     * @param declaration The variable declaration to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved variable declaration.
     */
    std::shared_ptr<VariableDeclaration> resolveLocalVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a statement.
     *
     * @param statement The statement to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved statement.
     */
    std::shared_ptr<Statement>
    resolveStatement(std::shared_ptr<Statement> statement,
                     std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve an expression.
     *
     * @param expression The expression to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved expression.
     */
    std::shared_ptr<Expression>
    resolveExpression(std::shared_ptr<Expression> expression,
                      std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a block.
     *
     * @param block The block to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved block.
     */
    std::shared_ptr<Block>
    resolveBlock(std::shared_ptr<Block> block,
                 std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a for-init.
     *
     * @param forInit The for-init to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved for-init.
     */
    std::shared_ptr<ForInit>
    resolveForInit(std::shared_ptr<ForInit> forInit,
                   std::unordered_map<std::string, MapEntry> &identifierMap);

    /**
     * Resolve a function declaration.
     *
     * @param declaration The function declaration to resolve.
     * @param identifierMap The current identifier map.
     * @return The resolved function declaration.
     */
    std::shared_ptr<FunctionDeclaration> resolveFunctionDeclaration(
        std::shared_ptr<FunctionDeclaration> declaration,
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
     * Default virtual destructor for static initializers.
     */
    virtual ~StaticInit() = default;

    /**
     * Pure virtual method to get the value of the static initializer.
     */
    virtual std::variant<int, long> getValue() = 0;
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

    std::variant<int, long> getValue() override { return value; }

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

    std::variant<int, long> getValue() override { return value; }

  private:
    /**
     * The long value of the static initializer.
     */
    long value;
};

/**
 * Base class for initial values in the AST.
 */
class InitialValue {
  public:
    /**
     * Default virtual destructor for initial values.
     */
    virtual ~InitialValue() = default;
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
     * Constructors for the initial class with an integer parameter.
     *
     * @param value The integer or long value of the static initializer.
     */
    Initial(int value) : staticInit(std::make_shared<IntInit>(value)) {}

    /**
     * Constructors for the initial class with a long parameter.
     *
     * @param value The long value of the static initializer.
     */
    Initial(long value) : staticInit(std::make_shared<LongInit>(value)) {}

    /**
     * Constructor for the initial class with a static initializer parameter.
     *
     * @param staticInit The static initializer.
     */
    Initial(std::shared_ptr<StaticInit> staticInit) : staticInit(staticInit) {}

    std::shared_ptr<StaticInit> getStaticInit() { return staticInit; }

  private:
    /**
     * The static initializer.
     */
    std::shared_ptr<StaticInit> staticInit;
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
     * Default virtual destructor for identifier attributes.
     */
    virtual ~IdentifierAttribute() = default;
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
    StaticAttribute(std::shared_ptr<InitialValue> initialValue, bool global)
        : initialValue(initialValue), global(global) {}

    std::shared_ptr<InitialValue> getInitialValue() { return initialValue; }

    constexpr bool isGlobal() { return global; }

  private:
    /**
     * The initial value of the static variable.
     */
    std::shared_ptr<InitialValue> initialValue;

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
    void typeCheckProgram(std::shared_ptr<Program> program);

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
    std::shared_ptr<StaticInit> convertStaticConstantToStaticInit(
        std::shared_ptr<Type> varType,
        std::shared_ptr<ConstantExpression> constantExpr);

    /**
     * Get the common type between two types.
     *
     * @param type1 The first type.
     * @param type2 The second type.
     */
    std::shared_ptr<Type> getCommonType(std::shared_ptr<Type> type1,
                                        std::shared_ptr<Type> type2);

    /**
     * Convert an expression to a target type.
     *
     * @param expression The expression to convert.
     * @param targetType The target type.
     */
    std::shared_ptr<Expression>
    convertTo(std::shared_ptr<Expression> expression,
              std::shared_ptr<Type> targetType);

    /**
     * Type check a function declaration.
     *
     * @param declaration The function declaration to type check.
     */
    void typeCheckFunctionDeclaration(
        std::shared_ptr<FunctionDeclaration> declaration);

    /**
     * Type check a file-scope variable declaration.
     *
     * @param declaration The variable declaration to type check.
     */
    void typeCheckFileScopeVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration);

    /**
     * Type check a local variable declaration.
     *
     * @param declaration The variable declaration to type check.
     */
    void typeCheckLocalVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration);

    /**
     * Type check an expression.
     *
     * @param expression The expression to type check.
     */
    void typeCheckExpression(std::shared_ptr<Expression> expression);

    /**
     * Type check a block.
     *
     * @param block The block to type check.
     * @param enclosingFunctionIdentifier The identifier of the enclosing
     * function.
     */
    void typeCheckBlock(std::shared_ptr<Block> block,
                        std::string enclosingFunctionIdentifier);

    /**
     * Type check a statement.
     *
     * @param statement The statement to type check.
     * @param enclosingFunctionIdentifier The identifier of the enclosing
     * function.
     */
    void typeCheckStatement(std::shared_ptr<Statement> statement,
                            std::string enclosingFunctionIdentifier);

    /**
     * Type check a for-init.
     *
     * @param forInit The for-init to type check.
     */
    void typeCheckForInit(std::shared_ptr<ForInit> forInit);
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
    void labelLoops(std::shared_ptr<Program> program);

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
     * @return The annotated statement.
     */
    std::shared_ptr<Statement>
    annotateStatement(std::shared_ptr<Statement> statement, std::string label);

    /**
     * Label a statement with a label.
     *
     * @param statement The statement to label.
     * @param label The label to use.
     * @return The labeled statement.
     */
    std::shared_ptr<Statement>
    labelStatement(std::shared_ptr<Statement> statement, std::string label);

    /**
     * Label a block with a label.
     *
     * @param block The block to label.
     * @param label The label to use.
     * @return The labeled block.
     */
    std::shared_ptr<Block> labelBlock(std::shared_ptr<Block> block,
                                      std::string label);
};
} // namespace AST

#endif // FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
