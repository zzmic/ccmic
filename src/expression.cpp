

#include "expression.h"
#include "visitor.h"

namespace AST {
ConstantExpression::ConstantExpression(int value) : value_(value) {}

void ConstantExpression::accept(Visitor &visitor) { visitor.visit(*this); }

int ConstantExpression::getValue() const { return value_; }

UnaryExpression::UnaryExpression(const std::string &op,
                                 std::shared_ptr<Expression> expr) {
    if (op == "-") {
        op_ = std::shared_ptr<NegateOperator>(new NegateOperator());
    }
    else if (op == "~") {
        op_ = std::shared_ptr<ComplementOperator>(new ComplementOperator());
    }
    expr_ = expr;
}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<UnaryOperator> UnaryExpression::getOperator() const {
    return op_;
}

std::shared_ptr<Expression> UnaryExpression::getExpression() const {
    return expr_;
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   const std::string &op,
                                   std::shared_ptr<Expression> right)
    : left_(left), op_(op), right_(right) {}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> BinaryExpression::getLeft() const { return left_; }

std::string BinaryExpression::getOperator() const { return op_; }

std::shared_ptr<Expression> BinaryExpression::getRight() const {
    return right_;
}
} // Namespace AST
