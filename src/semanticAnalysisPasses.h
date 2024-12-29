#ifndef SEMANTIC_ANALYSIS_PASSES_H
#define SEMANTIC_ANALYSIS_PASSES_H

#include "declaration.h"
#include "program.h"

#include <unordered_map>

namespace AST {
class VariableResolutionPass {
  public:
    void resolveVariables(std::shared_ptr<Program> program);

  private:
    std::unordered_map<std::string, std::string> variableMap;
    static std::string
    generateUniqueVariableName(const std::string &identifier);
    std::shared_ptr<Declaration>
    resolveVariableDeclaration(std::shared_ptr<Declaration> declaration);
    std::shared_ptr<Statement>
    resolveStatement(std::shared_ptr<Statement> statement);
    std::shared_ptr<Expression>
    resolveExpression(std::shared_ptr<Expression> expression);
};

} // namespace AST

#endif // SEMANTIC_ANALYSIS_PASSES_H
