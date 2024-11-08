

#include "statement.h"
#include "visitor.h"

namespace AST {
ReturnStatement::ReturnStatement(std::shared_ptr<Expression> expr)
    : expr_(expr) {}

void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ReturnStatement::getExpression() const {
    return expr_;
}
} // Namespace AST
