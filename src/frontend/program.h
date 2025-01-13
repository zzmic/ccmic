#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "ast.h"
#include "function.h"
#include <memory>

namespace AST {
class Program : public AST {
  public:
    Program(std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
                functionDeclarations);
    void accept(Visitor &visitor) override;
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
    getFunctionDeclarations() const;
    void setFunctionDeclarations(
        std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
            functionDeclarations);

  private:
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
        functionDeclarations;
};
} // Namespace AST

#endif // FRONTEND_PROGRAM_H
