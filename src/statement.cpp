#include "statement.h"
#include "visitor.h"

namespace AST {
ReturnStatement::ReturnStatement(std::shared_ptr<Expression> expr)
    : expr(expr) {}

void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ReturnStatement::getExpression() const {
    return expr;
}

ExpressionStatement::ExpressionStatement(std::shared_ptr<Expression> expr)
    : expr(expr) {}

void ExpressionStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ExpressionStatement::getExpression() const {
    return expr;
}

IfStatement::IfStatement(
    std::shared_ptr<Expression> condition,
    std::shared_ptr<Statement> thenStatement,
    std::optional<std::shared_ptr<Statement>> elseOptStatement)
    : condition(condition), thenStatement(thenStatement),
      elseOptStatement(elseOptStatement) {}

IfStatement::IfStatement(std::shared_ptr<Expression> condition,
                         std::shared_ptr<Statement> thenStatement)
    : condition(condition), thenStatement(thenStatement) {}

void IfStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> IfStatement::getCondition() const {
    return condition;
}

std::shared_ptr<Statement> IfStatement::getThenStatement() const {
    return thenStatement;
}

std::optional<std::shared_ptr<Statement>>
IfStatement::getElseOptStatement() const {
    return elseOptStatement;
}

void NullStatement::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
