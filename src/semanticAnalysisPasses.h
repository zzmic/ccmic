#ifndef SEMANTIC_ANALYSIS_PASSES_H
#define SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "program.h"

#include <unordered_map>

namespace AST {
class VariableResolutionPass {
  public:
    int resolveVariables(std::shared_ptr<Program> program);

  private:
    int variableResolutionCounter = 0;
    std::unordered_map<std::string, std::string> variableMap;
    std::string generateUniqueVariableName(const std::string &identifier);
    std::shared_ptr<Declaration>
    resolveVariableDeclaration(std::shared_ptr<Declaration> declaration);
    std::shared_ptr<Statement>
    resolveStatement(std::shared_ptr<Statement> statement);
    std::shared_ptr<Expression>
    resolveExpression(std::shared_ptr<Expression> expression);
};

} // namespace AST

#endif // SEMANTIC_ANALYSIS_PASSES_H
