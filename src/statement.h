#ifndef STATEMENT_H
#define STATEMENT_H

#include "ast.h"
#include "expression.h"
#include <memory>

namespace AST {
class Statement : public AST {};

class ReturnStatement : public Statement {
  public:
    ReturnStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Expression> expr;
};

class ExpressionStatement : public Statement {
  public:
    ExpressionStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Expression> expr;
};

class NullStatement : public Statement {
  public:
    NullStatement() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // STATEMENT_H
