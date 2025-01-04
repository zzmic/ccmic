#ifndef SEMANTIC_ANALYSIS_PASSES_H
#define SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "program.h"
#include <unordered_map>

namespace AST {
class MapEntry {
  public:
    MapEntry() : newName(""), fromCurrentBlock(false), hasLinkage(false) {}
    MapEntry(std::string newName, bool fromCurrentBlock, bool hasLinkage)
        : newName(newName), fromCurrentBlock(fromCurrentBlock),
          hasLinkage(hasLinkage) {}
    std::string getNewName() { return newName; }
    bool fromCurrentBlockOrNot() { return fromCurrentBlock; }
    bool hasLinkageOrNot() { return hasLinkage; }

  private:
    std::string newName;
    bool fromCurrentBlock;
    bool hasLinkage;
};

class IdentifierResolutionPass {
  public:
    int resolveIdentifiers(std::shared_ptr<Program> program);

  private:
    int variableResolutionCounter = 0;
    std::string generateUniqueVariableName(const std::string &identifier);
    std::unordered_map<std::string, MapEntry>
    copyIdentifierMap(std::unordered_map<std::string, MapEntry> &identifierMap);
    std::shared_ptr<VariableDeclaration> resolveVariableDeclaration(
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

class LoopLabelingPass {
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

#endif // SEMANTIC_ANALYSIS_PASSES_H
