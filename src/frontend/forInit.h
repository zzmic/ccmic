#ifndef FRONTEND_FOR_INIT_H
#define FRONTEND_FOR_INIT_H

#include "ast.h"
#include "declaration.h"
#include "expression.h"

namespace AST {
class ForInit : public AST {
  public:
    constexpr ForInit() = default;
};

class InitDecl : public ForInit {
  public:
    explicit InitDecl(std::unique_ptr<VariableDeclaration> decl);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<VariableDeclaration> &
    getVariableDeclaration();

  private:
    std::unique_ptr<VariableDeclaration> decl;
};

class InitExpr : public ForInit {
  public:
    constexpr InitExpr() = default;
    explicit InitExpr(std::optional<std::unique_ptr<Expression>> expr);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::optional<std::unique_ptr<Expression>> &getExpression();

  private:
    std::optional<std::unique_ptr<Expression>> expr;
};
} // Namespace AST

#endif // FRONTEND_FOR_INIT_H
