#ifndef FOR_INIT_H
#define FOR_INIT_H

#include "ast.h"
#include "declaration.h"
#include "expression.h"

namespace AST {
class ForInit : public AST {};

class InitDecl : public ForInit {
  public:
    InitDecl(std::shared_ptr<Declaration> decl);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Declaration> getDeclaration() const;

  private:
    std::shared_ptr<Declaration> decl;
};

class InitExpr : public ForInit {
  public:
    InitExpr() = default;
    InitExpr(std::optional<std::shared_ptr<Expression>> expr);
    void accept(Visitor &visitor) override;
    std::optional<std::shared_ptr<Expression>> getExpression() const;

  private:
    std::optional<std::shared_ptr<Expression>> expr;
};
} // Namespace AST

#endif // FOR_INIT_H
