#include "expression.h"
#include "visitor.h"

#include <stdexcept>

namespace AST {
ConstantExpression::ConstantExpression(std::shared_ptr<Constant> constant)
    : constant(constant) {}

ConstantExpression::ConstantExpression(std::shared_ptr<Constant> constant,
                                       std::shared_ptr<Type> expType)
    : constant(constant), expType(expType) {}

void ConstantExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Constant> ConstantExpression::getConstant() const {
    return constant;
}

std::shared_ptr<Type> ConstantExpression::getExpType() const { return expType; }

void ConstantExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

// TODO(zzmic): This is a temporary solution.
int ConstantExpression::getConstantInInt() const {
    if (auto intConst = std::dynamic_pointer_cast<ConstantInt>(constant)) {
        return intConst->getValue();
    }
    else if (auto longConst =
                 std::dynamic_pointer_cast<ConstantLong>(constant)) {
        return longConst->getValue();
    }
    else {
        throw std::runtime_error(
            "Unknown constant type in constant expression");
    }
}

VariableExpression::VariableExpression(const std::string &identifier)
    : identifier(identifier) {}

VariableExpression::VariableExpression(const std::string &identifier,
                                       std::shared_ptr<Type> expType)
    : identifier(identifier), expType(expType) {}

void VariableExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::string VariableExpression::getIdentifier() const { return identifier; }

std::shared_ptr<Type> VariableExpression::getExpType() const { return expType; }

void VariableExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

CastExpression::CastExpression(std::shared_ptr<Type> targetType,
                               std::shared_ptr<Expression> expr)
    : targetType(targetType), expr(expr) {}

CastExpression::CastExpression(std::shared_ptr<Type> targetType,
                               std::shared_ptr<Expression> expr,
                               std::shared_ptr<Type> expType)
    : targetType(targetType), expr(expr), expType(expType) {}

void CastExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Type> CastExpression::getTargetType() const {
    return targetType;
}

std::shared_ptr<Expression> CastExpression::getExpression() const {
    return expr;
}

std::shared_ptr<Type> CastExpression::getExpType() const { return expType; }

void CastExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

UnaryExpression::UnaryExpression(const std::string &opInStr,
                                 std::shared_ptr<Expression> expr) {
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
    if (!expr) {
        throw std::runtime_error("Null expression in unary expression");
    }
    // Static-cast the expression to a factor, obeying the grammar `<unop>
    // <factor>`.
    this->expr = std::static_pointer_cast<Factor>(expr);
}

UnaryExpression::UnaryExpression(const std::string &opInStr,
                                 std::shared_ptr<Expression> expr,
                                 std::shared_ptr<Type> expType)
    : expType(expType) {
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
    if (!expr) {
        throw std::runtime_error("Null expression in unary expression");
    }
    // Static-cast the expression to a factor, obeying the grammar `<unop>
    // <factor>`.
    this->expr = std::static_pointer_cast<Factor>(expr);
}

UnaryExpression::UnaryExpression(std::shared_ptr<UnaryOperator> op,
                                 std::shared_ptr<Factor> expr)
    : op(op), expr(expr) {}

UnaryExpression::UnaryExpression(std::shared_ptr<UnaryOperator> op,
                                 std::shared_ptr<Factor> expr,
                                 std::shared_ptr<Type> expType)
    : op(op), expr(expr), expType(expType) {}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<UnaryOperator> UnaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Factor> UnaryExpression::getExpression() const { return expr; }

std::shared_ptr<Type> UnaryExpression::getExpType() const { return expType; }

void UnaryExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   const std::string &opInStr,
                                   std::shared_ptr<Expression> right)
    : left(left), right(right) {
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
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   const std::string &opInStr,
                                   std::shared_ptr<Expression> right,
                                   std::shared_ptr<Type> expType)
    : left(left), right(right), expType(expType) {
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
}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   std::shared_ptr<BinaryOperator> op,
                                   std::shared_ptr<Expression> right)
    : left(left), op(op), right(right) {}

BinaryExpression::BinaryExpression(std::shared_ptr<Expression> left,
                                   std::shared_ptr<BinaryOperator> op,
                                   std::shared_ptr<Expression> right,
                                   std::shared_ptr<Type> expType)
    : left(left), op(op), right(right), expType(expType) {}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> BinaryExpression::getLeft() const { return left; }

std::shared_ptr<BinaryOperator> BinaryExpression::getOperator() const {
    return op;
}

std::shared_ptr<Expression> BinaryExpression::getRight() const { return right; }

std::shared_ptr<Type> BinaryExpression::getExpType() const { return expType; }

void BinaryExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

AssignmentExpression::AssignmentExpression(std::shared_ptr<Expression> left,
                                           std::shared_ptr<Expression> right)
    : left(left), right(right) {}

AssignmentExpression::AssignmentExpression(std::shared_ptr<Expression> left,
                                           std::shared_ptr<Expression> right,
                                           std::shared_ptr<Type> expType)
    : left(left), right(right), expType(expType) {}

void AssignmentExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> AssignmentExpression::getLeft() const {
    return left;
}

std::shared_ptr<Expression> AssignmentExpression::getRight() const {
    return right;
}

std::shared_ptr<Type> AssignmentExpression::getExpType() const {
    return expType;
}

void AssignmentExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

ConditionalExpression::ConditionalExpression(
    std::shared_ptr<Expression> condition,
    std::shared_ptr<Expression> thenExpression,
    std::shared_ptr<Expression> elseExpression)
    : condition(condition), thenExpression(thenExpression),
      elseExpression(elseExpression) {}

ConditionalExpression::ConditionalExpression(
    std::shared_ptr<Expression> condition,
    std::shared_ptr<Expression> thenExpression,
    std::shared_ptr<Expression> elseExpression, std::shared_ptr<Type> expType)
    : condition(condition), thenExpression(thenExpression),
      elseExpression(elseExpression), expType(expType) {}

void ConditionalExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ConditionalExpression::getCondition() const {
    return condition;
}

std::shared_ptr<Expression> ConditionalExpression::getThenExpression() const {
    return thenExpression;
}

std::shared_ptr<Expression> ConditionalExpression::getElseExpression() const {
    return elseExpression;
}

std::shared_ptr<Type> ConditionalExpression::getExpType() const {
    return expType;
}

void ConditionalExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}

FunctionCallExpression::FunctionCallExpression(
    const std::string &identifier,
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments)
    : identifier(identifier), arguments(arguments) {}

FunctionCallExpression::FunctionCallExpression(
    const std::string &identifier,
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments,
    std::shared_ptr<Type> expType)
    : identifier(identifier), arguments(arguments), expType(expType) {}

void FunctionCallExpression::accept(Visitor &visitor) { visitor.visit(*this); }

std::string FunctionCallExpression::getIdentifier() const { return identifier; }

std::shared_ptr<std::vector<std::shared_ptr<Expression>>>
FunctionCallExpression::getArguments() const {
    return arguments;
}

void FunctionCallExpression::setArguments(
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments) {
    this->arguments = arguments;
}

std::shared_ptr<Type> FunctionCallExpression::getExpType() const {
    return expType;
}

void FunctionCallExpression::setExpType(std::shared_ptr<Type> expType) {
    this->expType = expType;
}
} // Namespace AST
