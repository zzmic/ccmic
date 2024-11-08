

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
    std::shared_ptr<Expression> expr_;
};
} // Namespace AST

#endif // STATEMENT_H
