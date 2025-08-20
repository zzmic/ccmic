#include "expression.h"
#include "visitor.h"

#include <memory>
#include <stdexcept>

namespace AST {
ConstantExpression::ConstantExpression(std::unique_ptr<Constant> constant)
    : constant(std::move(constant)) {}

ConstantExpression::ConstantExpression(std::unique_ptr<Constant> constant,
                                       std::unique_ptr<Type> expType)
    : constant(std::move(constant)), expType(std::move(expType)) {}

void ConstantExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Constant> &ConstantExpression::getConstant() {
    return constant;
}

std::unique_ptr<Type> &ConstantExpression::getExpType() { return expType; }

void ConstantExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

std::variant<int, long>
ConstantExpression::getConstantInIntOrLongVariant() const {
    if (const auto *intConstant =
            dynamic_cast<const ConstantInt *>(constant.get())) {
        return intConstant->getValue();
    }
    else if (const auto *longConstant =
                 dynamic_cast<const ConstantLong *>(constant.get())) {
        return longConstant->getValue();
    }
    throw std::logic_error("Unknown constant type in constant expression");
}

VariableExpression::VariableExpression(std::string_view identifier)
    : identifier(identifier) {}

VariableExpression::VariableExpression(std::string_view identifier,
                                       std::unique_ptr<Type> expType)
    : identifier(identifier), expType(std::move(expType)) {}

void VariableExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &VariableExpression::getIdentifier() { return identifier; }

std::unique_ptr<Type> &VariableExpression::getExpType() { return expType; }

void VariableExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

CastExpression::CastExpression(std::unique_ptr<Type> targetType,
                               std::unique_ptr<Expression> expr)
    : targetType(std::move(targetType)), expr(std::move(expr)) {}

CastExpression::CastExpression(std::unique_ptr<Type> targetType,
                               std::unique_ptr<Expression> expr,
                               std::unique_ptr<Type> expType)
    : targetType(std::move(targetType)), expr(std::move(expr)),
      expType(std::move(expType)) {}

void CastExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Type> &CastExpression::getTargetType() { return targetType; }

std::unique_ptr<Expression> &CastExpression::getExpression() { return expr; }

std::unique_ptr<Type> &CastExpression::getExpType() { return expType; }

void CastExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

UnaryExpression::UnaryExpression(std::string_view opInStr,
                                 std::unique_ptr<Expression> expr) {
    if (opInStr == "-") {
        op = std::make_unique<NegateOperator>();
    }
    else if (opInStr == "~") {
        op = std::make_unique<ComplementOperator>();
    }
    else if (opInStr == "!") {
        op = std::make_unique<NotOperator>();
    }
    else {
        throw std::invalid_argument(
            "Invalid unary operator in unary expression");
    }
    if (!expr) {
        throw std::logic_error("Null expression in unary expression");
    }
    // Static-cast the expression to a factor, obeying the grammar `<unop>
    // <factor>`.
    this->expr = std::unique_ptr<Factor>(
        static_cast<Factor *>(std::move(expr).release()));
}

UnaryExpression::UnaryExpression(std::string_view opInStr,
                                 std::unique_ptr<Expression> expr,
                                 std::unique_ptr<Type> expType)
    : expType(std::move(expType)) {
    if (opInStr == "-") {
        op = std::make_unique<NegateOperator>();
    }
    else if (opInStr == "~") {
        op = std::make_unique<ComplementOperator>();
    }
    else if (opInStr == "!") {
        op = std::make_unique<NotOperator>();
    }
    else {
        throw std::invalid_argument(
            "Invalid unary operator in unary expression");
    }
    if (!expr) {
        throw std::logic_error("Null expression in unary expression");
    }
    // Static-cast the expression to a factor, obeying the grammar `<unop>
    // <factor>`.
    this->expr = std::unique_ptr<Factor>(static_cast<Factor *>(expr.release()));
}

UnaryExpression::UnaryExpression(std::unique_ptr<UnaryOperator> op,
                                 std::unique_ptr<Factor> expr)
    : op(std::move(op)), expr(std::move(expr)) {}

UnaryExpression::UnaryExpression(std::unique_ptr<UnaryOperator> op,
                                 std::unique_ptr<Factor> expr,
                                 std::unique_ptr<Type> expType)
    : op(std::move(op)), expr(std::move(expr)), expType(std::move(expType)) {}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<UnaryOperator> &UnaryExpression::getOperator() { return op; }

std::unique_ptr<Factor> &UnaryExpression::getExpression() { return expr; }

void UnaryExpression::setExpression(std::unique_ptr<Factor> newExpr) {
    this->expr = std::move(newExpr);
}

std::unique_ptr<Type> &UnaryExpression::getExpType() { return expType; }

void UnaryExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::string_view opInStr,
                                   std::unique_ptr<Expression> right)
    : left(std::move(left)), right(std::move(right)) {
    if (opInStr == "+") {
        op = std::make_unique<AddOperator>();
    }
    else if (opInStr == "-") {
        op = std::make_unique<SubtractOperator>();
    }
    else if (opInStr == "*") {
        op = std::make_unique<MultiplyOperator>();
    }
    else if (opInStr == "/") {
        op = std::make_unique<DivideOperator>();
    }
    else if (opInStr == "%") {
        op = std::make_unique<RemainderOperator>();
    }
    else if (opInStr == "&&") {
        op = std::make_unique<AndOperator>();
    }
    else if (opInStr == "||") {
        op = std::make_unique<OrOperator>();
    }
    else if (opInStr == "==") {
        op = std::make_unique<EqualOperator>();
    }
    else if (opInStr == "!=") {
        op = std::make_unique<NotEqualOperator>();
    }
    else if (opInStr == "<") {
        op = std::make_unique<LessThanOperator>();
    }
    else if (opInStr == "<=") {
        op = std::make_unique<LessThanOrEqualOperator>();
    }
    else if (opInStr == ">") {
        op = std::make_unique<GreaterThanOperator>();
    }
    else if (opInStr == ">=") {
        op = std::make_unique<GreaterThanOrEqualOperator>();
    }
    else {
        throw std::invalid_argument(
            "Invalid binary operator in binary expression");
    }
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::string_view opInStr,
                                   std::unique_ptr<Expression> right,
                                   std::unique_ptr<Type> expType)
    : left(std::move(left)), right(std::move(right)),
      expType(std::move(expType)) {
    if (opInStr == "+") {
        op = std::make_unique<AddOperator>();
    }
    else if (opInStr == "-") {
        op = std::make_unique<SubtractOperator>();
    }
    else if (opInStr == "*") {
        op = std::make_unique<MultiplyOperator>();
    }
    else if (opInStr == "/") {
        op = std::make_unique<DivideOperator>();
    }
    else if (opInStr == "%") {
        op = std::make_unique<RemainderOperator>();
    }
    else if (opInStr == "&&") {
        op = std::make_unique<AndOperator>();
    }
    else if (opInStr == "||") {
        op = std::make_unique<OrOperator>();
    }
    else if (opInStr == "==") {
        op = std::make_unique<EqualOperator>();
    }
    else if (opInStr == "!=") {
        op = std::make_unique<NotEqualOperator>();
    }
    else if (opInStr == "<") {
        op = std::make_unique<LessThanOperator>();
    }
    else if (opInStr == "<=") {
        op = std::make_unique<LessThanOrEqualOperator>();
    }
    else if (opInStr == ">") {
        op = std::make_unique<GreaterThanOperator>();
    }
    else if (opInStr == ">=") {
        op = std::make_unique<GreaterThanOrEqualOperator>();
    }
    else {
        throw std::invalid_argument(
            "Invalid binary operator in binary expression");
    }
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::unique_ptr<BinaryOperator> op,
                                   std::unique_ptr<Expression> right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::unique_ptr<BinaryOperator> op,
                                   std::unique_ptr<Expression> right,
                                   std::unique_ptr<Type> expType)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)),
      expType(std::move(expType)) {}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &BinaryExpression::getLeft() { return left; }

void BinaryExpression::setLeft(std::unique_ptr<Expression> newLeft) {
    this->left = std::move(newLeft);
}

std::unique_ptr<BinaryOperator> &BinaryExpression::getOperator() { return op; }

void BinaryExpression::setOperator(std::unique_ptr<BinaryOperator> newOp) {
    this->op = std::move(newOp);
}

std::unique_ptr<Expression> &BinaryExpression::getRight() { return right; }

void BinaryExpression::setRight(std::unique_ptr<Expression> newRight) {
    this->right = std::move(newRight);
}

std::unique_ptr<Type> &BinaryExpression::getExpType() { return expType; }

void BinaryExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> left,
                                           std::unique_ptr<Expression> right)
    : left(std::move(left)), right(std::move(right)) {}

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> left,
                                           std::unique_ptr<Expression> right,
                                           std::unique_ptr<Type> expType)
    : left(std::move(left)), right(std::move(right)),
      expType(std::move(expType)) {}

void AssignmentExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &AssignmentExpression::getLeft() { return left; }

void AssignmentExpression::setLeft(std::unique_ptr<Expression> newLeft) {
    this->left = std::move(newLeft);
}

std::unique_ptr<Expression> &AssignmentExpression::getRight() { return right; }

void AssignmentExpression::setRight(std::unique_ptr<Expression> newRight) {
    this->right = std::move(newRight);
}

std::unique_ptr<Type> &AssignmentExpression::getExpType() { return expType; }

void AssignmentExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

ConditionalExpression::ConditionalExpression(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Expression> thenExpression,
    std::unique_ptr<Expression> elseExpression)
    : condition(std::move(condition)),
      thenExpression(std::move(thenExpression)),
      elseExpression(std::move(elseExpression)) {}

ConditionalExpression::ConditionalExpression(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Expression> thenExpression,
    std::unique_ptr<Expression> elseExpression, std::unique_ptr<Type> expType)
    : condition(std::move(condition)),
      thenExpression(std::move(thenExpression)),
      elseExpression(std::move(elseExpression)), expType(std::move(expType)) {}

void ConditionalExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &ConditionalExpression::getCondition() {
    return condition;
}

void ConditionalExpression::setCondition(
    std::unique_ptr<Expression> newCondition) {
    this->condition = std::move(newCondition);
}

std::unique_ptr<Expression> &ConditionalExpression::getThenExpression() {
    return thenExpression;
}

void ConditionalExpression::setThenExpression(
    std::unique_ptr<Expression> newThenExpression) {
    this->thenExpression = std::move(newThenExpression);
}

std::unique_ptr<Expression> &ConditionalExpression::getElseExpression() {
    return elseExpression;
}

void ConditionalExpression::setElseExpression(
    std::unique_ptr<Expression> newElseExpression) {
    this->elseExpression = std::move(newElseExpression);
}

std::unique_ptr<Type> &ConditionalExpression::getExpType() { return expType; }

void ConditionalExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}

FunctionCallExpression::FunctionCallExpression(
    std::string_view identifier,
    std::vector<std::unique_ptr<Expression>> arguments)
    : identifier(identifier), arguments(std::move(arguments)) {}

FunctionCallExpression::FunctionCallExpression(
    std::string_view identifier,
    std::vector<std::unique_ptr<Expression>> arguments,
    std::unique_ptr<Type> expType)
    : identifier(identifier), arguments(std::move(arguments)),
      expType(std::move(expType)) {}

void FunctionCallExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &FunctionCallExpression::getIdentifier() { return identifier; }

std::vector<std::unique_ptr<Expression>> &
FunctionCallExpression::getArguments() {
    return arguments;
}

void FunctionCallExpression::setArguments(
    std::vector<std::unique_ptr<Expression>> newArguments) {
    this->arguments = std::move(newArguments);
}

std::unique_ptr<Type> &FunctionCallExpression::getExpType() { return expType; }

void FunctionCallExpression::setExpType(std::unique_ptr<Type> newExpType) {
    this->expType = std::move(newExpType);
}
} // Namespace AST
