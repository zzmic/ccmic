#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "ast.h"
#include "function.h"
#include <memory>

namespace AST {
class Program : public AST {
  public:
    explicit Program(std::vector<std::unique_ptr<Declaration>> declarations);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::vector<std::unique_ptr<Declaration>> &getDeclarations();
    void
    setDeclarations(std::vector<std::unique_ptr<Declaration>> newDeclarations);

  private:
    std::vector<std::unique_ptr<Declaration>> declarations;
};
} // Namespace AST

#endif // FRONTEND_PROGRAM_H
