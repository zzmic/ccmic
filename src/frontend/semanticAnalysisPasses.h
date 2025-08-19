#ifndef FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
#define FRONTEND_SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "expression.h"
#include "program.h"
#include "statement.h"
#include "type.h"
#include <unordered_map>
#include <variant>

namespace AST {
class SemanticAnalysisPass {
  public:
    virtual ~SemanticAnalysisPass() = default;
};

class MapEntry {
  public:
    MapEntry() : newName(""), fromCurrentScope(false), hasLinkage(false) {}
    MapEntry(std::string newName, bool fromCurrentScope, bool hasLinkage)
        : newName(newName), fromCurrentScope(fromCurrentScope),
          hasLinkage(hasLinkage) {}
    [[nodiscard]] std::string &getNewName() { return newName; }
    [[nodiscard]] constexpr bool fromCurrentScopeOrNot() {
        return fromCurrentScope;
    }
    [[nodiscard]] constexpr bool hasLinkageOrNot() { return hasLinkage; }

  private:
    std::string newName;
    bool fromCurrentScope;
    bool hasLinkage;
};

class IdentifierResolutionPass : public SemanticAnalysisPass {
  public:
    [[nodiscard]] int resolveProgram(std::unique_ptr<Program> program);

  private:
    int variableResolutionCounter = 0;
    [[nodiscard]] std::string
    generateUniqueVariableName(const std::string &identifier);
    [[nodiscard]] std::unordered_map<std::string, MapEntry>
    copyIdentifierMap(std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<Declaration>
    resolveFileScopeVariableDeclaration(
        std::unique_ptr<Declaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<VariableDeclaration>
    resolveLocalVariableDeclaration(
        std::unique_ptr<VariableDeclaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<Statement>
    resolveStatement(std::unique_ptr<Statement> statement,
                     std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<Expression>
    resolveExpression(std::unique_ptr<Expression> expression,
                      std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] Block *
    resolveBlock(Block *block,
                 std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<ForInit>
    resolveForInit(std::unique_ptr<ForInit> forInit,
                   std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::unique_ptr<FunctionDeclaration>
    resolveFunctionDeclaration(
        std::unique_ptr<FunctionDeclaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    [[nodiscard]] std::string
    resolveParameter(std::string parameter,
                     std::unordered_map<std::string, MapEntry> &identifierMap);
};

class StaticInit {
  public:
    virtual ~StaticInit() = default;
    [[nodiscard]] virtual std::variant<int, long> getValue() const = 0;
};

class IntInit : public StaticInit {
  public:
    constexpr IntInit(int value) : value(value) {}
    [[nodiscard]] std::variant<int, long> getValue() const override {
        return value;
    }

  private:
    int value;
};

class LongInit : public StaticInit {
  public:
    constexpr LongInit(long value) : value(value) {}
    [[nodiscard]] std::variant<int, long> getValue() const override {
        return value;
    }

  private:
    long value;
};

class InitialValue {
  public:
    virtual ~InitialValue() = default;
};

class Tentative : public InitialValue {};

class Initial : public InitialValue {
  public:
    Initial(int value) : staticInit(std::make_unique<IntInit>(value)) {}
    Initial(long value) : staticInit(std::make_unique<LongInit>(value)) {}
    Initial(std::unique_ptr<StaticInit> staticInit)
        : staticInit(std::move(staticInit)) {}
    [[nodiscard]] std::unique_ptr<StaticInit> &getStaticInit() {
        return staticInit;
    }

  private:
    std::unique_ptr<StaticInit> staticInit;
};

class NoInitializer : public InitialValue {};

class IdentifierAttribute {
  public:
    virtual ~IdentifierAttribute() = default;
};

class FunctionAttribute : public IdentifierAttribute {
  public:
    constexpr FunctionAttribute(bool defined, bool global)
        : defined(defined), global(global) {}
    [[nodiscard]] constexpr bool isDefined() { return defined; }
    [[nodiscard]] constexpr bool isGlobal() { return global; }

  private:
    bool defined;
    bool global;
};

class StaticAttribute : public IdentifierAttribute {
  public:
    StaticAttribute(std::unique_ptr<InitialValue> initialValue, bool global)
        : initialValue(std::move(initialValue)), global(global) {}
    [[nodiscard]] std::unique_ptr<InitialValue> &getInitialValue() {
        return initialValue;
    }
    [[nodiscard]] constexpr bool isGlobal() { return global; }

  private:
    std::unique_ptr<InitialValue> initialValue;
    bool global;
};

class LocalAttribute : public IdentifierAttribute {};

class TypeCheckingPass : public SemanticAnalysisPass {
  public:
    [[nodiscard]] std::unique_ptr<Program>
    typeCheckProgram(std::unique_ptr<Program> program);

  private:
    // Convert a compile-time constant (int or long) to a static initializer
    // (`IntInit` or `LongInit`).
    [[nodiscard]] std::unique_ptr<StaticInit> convertStaticConstantToStaticInit(
        std::unique_ptr<Type> varType,
        std::unique_ptr<ConstantExpression> constantExpr);
    [[nodiscard]] std::unique_ptr<Type>
    getCommonType(std::unique_ptr<Type> type1, std::unique_ptr<Type> type2);
    [[nodiscard]] std::unique_ptr<Expression>
    convertTo(std::unique_ptr<Expression> expression,
              std::unique_ptr<Type> targetType);
    [[nodiscard]] std::unique_ptr<FunctionDeclaration>
    typeCheckFunctionDeclaration(
        std::unique_ptr<FunctionDeclaration> declaration);
    [[nodiscard]] std::unique_ptr<VariableDeclaration>
    typeCheckFileScopeVariableDeclaration(
        std::unique_ptr<VariableDeclaration> declaration);
    [[nodiscard]] std::unique_ptr<VariableDeclaration>
    typeCheckLocalVariableDeclaration(
        std::unique_ptr<VariableDeclaration> declaration);
    [[nodiscard]] std::unique_ptr<Expression>
    typeCheckExpression(std::unique_ptr<Expression> expression);
    [[nodiscard]] Block *
    typeCheckBlock(Block *block, std::string enclosingFunctionIdentifier);
    [[nodiscard]] std::unique_ptr<Statement>
    typeCheckStatement(std::unique_ptr<Statement> statement,
                       std::string enclosingFunctionIdentifier);
    [[nodiscard]] std::unique_ptr<ForInit>
    typeCheckForInit(std::unique_ptr<ForInit> forInit);
};

class LoopLabelingPass : public SemanticAnalysisPass {
  public:
    [[nodiscard]] std::unique_ptr<Program>
    labelLoops(std::unique_ptr<Program> program);

  private:
    int loopLabelingCounter = 0;
    [[nodiscard]] std::string generateLoopLabel();
    [[nodiscard]] std::unique_ptr<Statement>
    annotateStatement(std::unique_ptr<Statement> statement, std::string label);
    [[nodiscard]] std::unique_ptr<Statement>
    labelStatement(std::unique_ptr<Statement> statement, std::string label);
    [[nodiscard]] Block *labelBlock(Block *block, std::string label);
};
} // namespace AST

#endif // FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
