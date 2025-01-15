#ifndef FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
#define FRONTEND_SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "expression.h"
#include "program.h"
#include "statement.h"
#include "types.h"
#include <unordered_map>

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
    std::string getNewName() { return newName; }
    bool fromCurrentScopeOrNot() { return fromCurrentScope; }
    bool hasLinkageOrNot() { return hasLinkage; }

  private:
    std::string newName;
    bool fromCurrentScope;
    bool hasLinkage;
};

class IdentifierResolutionPass : public SemanticAnalysisPass {
  public:
    int resolveProgram(std::shared_ptr<Program> program);

  private:
    int variableResolutionCounter = 0;
    std::string generateUniqueVariableName(const std::string &identifier);
    std::unordered_map<std::string, MapEntry>
    copyIdentifierMap(std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<Declaration> resolveFileScopeVariableDeclaration(
        std::shared_ptr<Declaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<VariableDeclaration> resolveLocalVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<Statement>
    resolveStatement(std::shared_ptr<Statement> statement,
                     std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<Expression>
    resolveExpression(std::shared_ptr<Expression> expression,
                      std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<Block>
    resolveBlock(std::shared_ptr<Block> block,
                 std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<ForInit>
    resolveForInit(std::shared_ptr<ForInit> forInit,
                   std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<FunctionDeclaration> resolveFunctionDeclaration(
        std::shared_ptr<FunctionDeclaration> declaration,
        std::unordered_map<std::string, MapEntry> &identifierMap);
    std::string
    resolveParameter(std::string parameter,
                     std::unordered_map<std::string, MapEntry> &identifierMap);
};

class InitialValue {
  public:
    virtual ~InitialValue() = default;
};

class Tentative : public InitialValue {};

class ConstantInitial : public InitialValue {
  public:
    ConstantInitial(int value) : value(value) {}
    int getValue() { return value; }

  private:
    int value;
};

class NoInitializer : public InitialValue {};

class IdentifierAttribute {
  public:
    virtual ~IdentifierAttribute() = default;
};

class FunctionAttribute : public IdentifierAttribute {
  public:
    FunctionAttribute(bool defined, bool global)
        : defined(defined), global(global) {}
    bool isDefined() { return defined; }
    bool isGlobal() { return global; }

  private:
    bool defined;
    bool global;
};

class StaticAttribute : public IdentifierAttribute {
  public:
    StaticAttribute(std::shared_ptr<InitialValue> initialValue, bool global)
        : initialValue(initialValue), global(global) {}
    std::shared_ptr<InitialValue> getInitialValue() { return initialValue; }
    bool isGlobal() { return global; }

  private:
    std::shared_ptr<InitialValue> initialValue;
    bool global;
};

class LocalAttribute : public IdentifierAttribute {};

class TypeCheckingPass : public SemanticAnalysisPass {
  public:
    std::unordered_map<
        std::string,
        std::pair<std::shared_ptr<Type>, std::shared_ptr<IdentifierAttribute>>>
    typeCheckProgram(std::shared_ptr<Program> program);

  private:
    std::unordered_map<
        std::string,
        std::pair<std::shared_ptr<Type>, std::shared_ptr<IdentifierAttribute>>>
        symbols;
    void typeCheckFunctionDeclaration(
        std::shared_ptr<FunctionDeclaration> declaration);
    void typeCheckFileScopeVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration);
    void typeCheckLocalVariableDeclaration(
        std::shared_ptr<VariableDeclaration> declaration);
    void typeCheckExpression(std::shared_ptr<Expression> expression);
    void typeCheckBlock(std::shared_ptr<Block> block);
    void typeCheckStatement(std::shared_ptr<Statement> statement);
    void typeCheckForInit(std::shared_ptr<ForInit> forInit);
};

class LoopLabelingPass : public SemanticAnalysisPass {
  public:
    void labelLoops(std::shared_ptr<Program> program);

  private:
    int loopLabelingCounter = 0;
    std::string generateLoopLabel();
    std::shared_ptr<Statement>
    annotateStatement(std::shared_ptr<Statement> statement, std::string label);
    std::shared_ptr<Statement>
    labelStatement(std::shared_ptr<Statement> statement, std::string label);
    std::shared_ptr<Block> labelBlock(std::shared_ptr<Block> block,
                                      std::string label);
};
} // namespace AST

#endif // FRONTEND_SEMANTIC_ANALYSIS_PASSES_H
