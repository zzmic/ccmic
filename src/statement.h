#ifndef STATEMENT_H
#define STATEMENT_H

#include "ast.h"
#include "expression.h"
#include <memory>
#include <optional>

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

class IfStatement : public Statement {
  public:
    IfStatement(std::shared_ptr<Expression> condition,
                std::shared_ptr<Statement> thenStatement,
                std::optional<std::shared_ptr<Statement>> elseOptStatement);
    IfStatement(std::shared_ptr<Expression> condition,
                std::shared_ptr<Statement> thenStatement);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Statement> getThenStatement() const;
    std::optional<std::shared_ptr<Statement>> getElseOptStatement() const;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenStatement;
    std::optional<std::shared_ptr<Statement>> elseOptStatement;
};

class NullStatement : public Statement {
  public:
    NullStatement() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // STATEMENT_H
