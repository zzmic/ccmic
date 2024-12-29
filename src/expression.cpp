#include "expression.h"
#include "visitor.h"

#include <stdexcept>

namespace AST {
ConstantExpression::ConstantExpression(int value) : value(value) {}

void ConstantExpression::accept(Visitor &visitor) { visitor.visit(*this); }

int ConstantExpression::getValue() const { return value; }

VariableExpression::VariableExpression(const std::string &identifier)
    : identifier(identifier) {}

void VariableExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::string VariableExpression::getIdentifier() const { return identifier; }

UnaryExpression::UnaryExpression(const std::string &opInStr,
                                 std::shared_ptr<Expression> expr) {
    if (!expr) {
        throw std::runtime_error("Null expression in unary expression");
    }
    if (opInStr == "-") {
        op = std::make_shared<NegateOperator>();
    }
    else if (opInStr == "~") {
        op = std::make_shared<ComplementOperator>();
    }
    else if (opInStr == "!") {
        op = std::make_shared<NotOperator>();
    }
    else {
        throw std::runtime_error("Invalid unary operator in unary expression");
    }
    // Static-cast the expression to a factor, obeying the grammar `<unop>
    // <factor>`.
    this->expr = std::static_pointer_cast<Factor>(expr);
}

UnaryExpression::UnaryExpression(std::shared_ptr<UnaryOperator> op,
                                 std::shared_ptr<Factor> expr)
    : op(op), expr(expr) {}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<UnaryOperator> UnaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Factor> UnaryExpression::getExpression() const { return expr; }

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   const std::string &opInStr,
                                   std::shared_ptr<Expression> right) {
    if (!left) {
        throw std::runtime_error("Null left-hand operand in binary expression");
    }
    if (!right) {
        throw std::runtime_error(
            "Null right-hand operand in binary expression");
    }
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
    else if (opInStr == "&&") {
        op = std::make_shared<AndOperator>();
    }
    else if (opInStr == "||") {
        op = std::make_shared<OrOperator>();
    }
    else if (opInStr == "==") {
        op = std::make_shared<EqualOperator>();
    }
    else if (opInStr == "!=") {
        op = std::make_shared<NotEqualOperator>();
    }
    else if (opInStr == "<") {
        op = std::make_shared<LessThanOperator>();
    }
    else if (opInStr == "<=") {
        op = std::make_shared<LessThanOrEqualOperator>();
    }
    else if (opInStr == ">") {
        op = std::make_shared<GreaterThanOperator>();
    }
    else if (opInStr == ">=") {
        op = std::make_shared<GreaterThanOrEqualOperator>();
    }
    else {
        throw std::runtime_error(
            "Invalid binary operator in binary expression");
    }
    this->left = left;
    this->right = right;
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   std::shared_ptr<BinaryOperator> op,
                                   std::shared_ptr<Expression> right)
    : left(left), op(op), right(right) {}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> BinaryExpression::getLeft() const { return left; }

std::shared_ptr<BinaryOperator> BinaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Expression> BinaryExpression::getRight() const { return right; }

AssignmentExpression::AssignmentExpression(std::shared_ptr<Expression> left,
                                           std::shared_ptr<Expression> right) {
    if (!left) {
        throw std::runtime_error("Null left-hand operand in assignment");
    }
    if (!right) {
        throw std::runtime_error("Null right-hand operand in assignment");
    }
    this->left = left;
    this->right = right;
}

void AssignmentExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> AssignmentExpression::getLeft() const {
    return left;
}

std::shared_ptr<Expression> AssignmentExpression::getRight() const {
    return right;
}
} // Namespace AST
