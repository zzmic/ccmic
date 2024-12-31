#ifndef SEMANTIC_ANALYSIS_PASSES_H
#define SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "program.h"
#include <unordered_map>

namespace AST {
class MapEntry {
  public:
    MapEntry(std::string newName, bool fromCurrentBlock)
        : newName(newName), fromCurrentBlock(fromCurrentBlock) {}
    MapEntry() : newName(""), fromCurrentBlock(false) {}
    std::string getNewName() { return newName; }
    bool isFromCurrentBlock() { return fromCurrentBlock; }

  private:
    std::string newName;
    bool fromCurrentBlock;
};

class VariableResolutionPass {
  public:
    int resolveVariables(std::shared_ptr<Program> program);

  private:
    int variableResolutionCounter = 0;
    std::string generateUniqueVariableName(const std::string &identifier);
    std::unordered_map<std::string, MapEntry>
    copyVariableMap(std::unordered_map<std::string, MapEntry> &variableMap);
    std::shared_ptr<Declaration> resolveVariableDeclaration(
        std::shared_ptr<Declaration> declaration,
        std::unordered_map<std::string, MapEntry> &variableMap);
    std::shared_ptr<Statement>
    resolveStatement(std::shared_ptr<Statement> statement,
                     std::unordered_map<std::string, MapEntry> &variableMap);
    std::shared_ptr<Expression>
    resolveExpression(std::shared_ptr<Expression> expression,
                      std::unordered_map<std::string, MapEntry> &variableMap);
    std::shared_ptr<Block>
    resolveBlock(std::shared_ptr<Block> block,
                 std::unordered_map<std::string, MapEntry> &variableMap);
};
} // namespace AST

#endif // SEMANTIC_ANALYSIS_PASSES_H
