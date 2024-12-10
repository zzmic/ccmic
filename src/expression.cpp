#include "expression.h"
#include "visitor.h"

namespace AST {
IntegerExpression::IntegerExpression(int value) : value(value) {}

void IntegerExpression::accept(Visitor &visitor) { visitor.visit(*this); }

int IntegerExpression::getValue() const { return value; }

UnaryExpression::UnaryExpression(const std::string &opInStr,
                                 std::shared_ptr<Expression> expr) {
    if (opInStr == "-") {
        op = std::make_shared<NegateOperator>();
    }
    else if (opInStr == "~") {
        op = std::make_shared<ComplementOperator>();
    }
    // Static-cast the expression to a factor.
    this->expr = std::static_pointer_cast<Factor>(expr);
}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<UnaryOperator> UnaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Factor> UnaryExpression::getExpression() const { return expr; }

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   const std::string &opInStr,
                                   std::shared_ptr<Expression> right) {
    if (opInStr == "+") {
        op = std::make_shared<AddOperator>();
    }
    else if (opInStr == "-") {
        op = std::make_shared<SubtractOperator>();
    }
    else if (opInStr == "*") {
        op = std::make_shared<MultiplyOperator>();
    }
    else if (opInStr == "/") {
        op = std::make_shared<DivideOperator>();
    }
    else if (opInStr == "%") {
        op = std::make_shared<RemainderOperator>();
    }
    this->left = left;
    this->right = right;
}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> BinaryExpression::getLeft() const { return left; }

std::shared_ptr<BinaryOperator> BinaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Expression> BinaryExpression::getRight() const { return right; }
} // Namespace AST
