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

void NullStatement::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
