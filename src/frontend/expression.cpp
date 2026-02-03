#include "expression.h"
#include "visitor.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace AST {
ConstantExpression::ConstantExpression(std::unique_ptr<Constant> constant)
    : constant(std::move(constant)) {
    if (!this->constant) {
        throw std::invalid_argument(
            "Creating ConstantExpression with null constant "
            "in ConstantExpression");
    }
}

ConstantExpression::ConstantExpression(std::unique_ptr<Constant> constant,
                                       std::unique_ptr<Type> expType)
    : constant(std::move(constant)), expType(std::move(expType)) {
    if (!this->constant) {
        throw std::invalid_argument(
            "Creating ConstantExpression with null constant "
            "in ConstantExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating ConstantExpression with null expType "
            "in ConstantExpression");
    }
}

void ConstantExpression::accept(Visitor &visitor) { visitor.visit(*this); }

Constant *ConstantExpression::getConstant() const { return constant.get(); }

Type *ConstantExpression::getExpType() const { return expType.get(); }

void ConstantExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

std::variant<int, long, unsigned int, unsigned long>
ConstantExpression::getConstantInVariant() const {
    if (auto *intConstant = dynamic_cast<ConstantInt *>(constant.get())) {
        return intConstant->getValue();
    }
    else if (auto *longConstant =
                 dynamic_cast<ConstantLong *>(constant.get())) {
        return longConstant->getValue();
    }
    else if (auto *uintConstant =
                 dynamic_cast<ConstantUInt *>(constant.get())) {
        return static_cast<unsigned int>(uintConstant->getValue());
    }
    else if (auto *ulongConstant =
                 dynamic_cast<ConstantULong *>(constant.get())) {
        return static_cast<unsigned long>(ulongConstant->getValue());
    }
    else {
        const auto &r = *constant;
        throw std::logic_error("Unsupported constant type in constant "
                               "expression in ConstantExpression: " +
                               std::string(typeid(r).name()));
    }
}

VariableExpression::VariableExpression(std::string_view identifier)
    : identifier(identifier) {}

VariableExpression::VariableExpression(std::string_view identifier,
                                       std::unique_ptr<Type> expType)
    : identifier(identifier), expType(std::move(expType)) {
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating VariableExpression with null expType "
            "in VariableExpression");
    }
}

void VariableExpression::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &VariableExpression::getIdentifier() const {
    return identifier;
}

void VariableExpression::setIdentifier(std::string_view newIdentifier) {
    identifier = newIdentifier;
}

Type *VariableExpression::getExpType() const { return expType.get(); }

void VariableExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

CastExpression::CastExpression(std::unique_ptr<Type> targetType,
                               std::unique_ptr<Expression> expr)
    : targetType(std::move(targetType)), expr(std::move(expr)) {
    if (!this->targetType) {
        throw std::invalid_argument(
            "Creating CastExpression with null targetType in CastExpression");
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating CastExpression with null expr in CastExpression");
    }
}

CastExpression::CastExpression(std::unique_ptr<Type> targetType,
                               std::unique_ptr<Expression> expr,
                               std::unique_ptr<Type> expType)
    : targetType(std::move(targetType)), expr(std::move(expr)),
      expType(std::move(expType)) {
    if (!this->targetType) {
        throw std::invalid_argument(
            "Creating CastExpression with null targetType in CastExpression");
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating CastExpression with null expr in CastExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating CastExpression with null expType in CastExpression");
    }
}

void CastExpression::accept(Visitor &visitor) { visitor.visit(*this); }

Type *CastExpression::getTargetType() const { return targetType.get(); }

Expression *CastExpression::getExpression() const { return expr.get(); }

Type *CastExpression::getExpType() const { return expType.get(); }

void CastExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

UnaryExpression::UnaryExpression(std::string_view opInStr,
                                 std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {
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
        throw std::invalid_argument("Creating UnaryExpression with invalid "
                                    "unary operator in UnaryExpression: " +
                                    std::string(opInStr));
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression in UnaryExpression");
    }
}

UnaryExpression::UnaryExpression(std::string_view opInStr,
                                 std::unique_ptr<Expression> expr,
                                 std::unique_ptr<Type> expType)
    : expr(std::move(expr)), expType(std::move(expType)) {
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
        throw std::invalid_argument("Creating UnaryExpression with invalid "
                                    "unary operator in UnaryExpression: " +
                                    std::string(opInStr));
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression in UnaryExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression "
            "type in UnaryExpression");
    }
}

UnaryExpression::UnaryExpression(std::unique_ptr<UnaryOperator> op,
                                 std::unique_ptr<Expression> expr)
    : op(std::move(op)), expr(std::move(expr)) {
    if (!this->op) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null operator in UnaryExpression");
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression in UnaryExpression");
    }
}

UnaryExpression::UnaryExpression(std::unique_ptr<UnaryOperator> op,
                                 std::unique_ptr<Expression> expr,
                                 std::unique_ptr<Type> expType)
    : op(std::move(op)), expr(std::move(expr)), expType(std::move(expType)) {
    if (!this->op) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null operator in UnaryExpression");
    }
    if (!this->expr) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression in UnaryExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating UnaryExpression with null expression "
            "type in UnaryExpression");
    }
}

void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

UnaryOperator *UnaryExpression::getOperator() const { return op.get(); }

Expression *UnaryExpression::getExpression() const { return expr.get(); }

Type *UnaryExpression::getExpType() const { return expType.get(); }

void UnaryExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
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
        throw std::invalid_argument("Creating BinaryExpression with invalid "
                                    "binary operator in BinaryExpression: " +
                                    std::string(opInStr));
    }
    if (!this->left) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null left in BinaryExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null right in BinaryExpression");
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
        throw std::invalid_argument("Creating BinaryExpression with invalid "
                                    "binary operator in BinaryExpression: " +
                                    std::string(opInStr));
    }
    if (!this->left) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null left in BinaryExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null right in BinaryExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null expType in BinaryExpression");
    }
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::unique_ptr<BinaryOperator> op,
                                   std::unique_ptr<Expression> right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {
    if (!this->left) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null left in BinaryExpression");
    }
    if (!this->op) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null op in BinaryExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null right in BinaryExpression");
    }
}

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::unique_ptr<BinaryOperator> op,
                                   std::unique_ptr<Expression> right,
                                   std::unique_ptr<Type> expType)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)),
      expType(std::move(expType)) {
    if (!this->left) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null left in BinaryExpression");
    }
    if (!this->op) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null op in BinaryExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null right in BinaryExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument(
            "Creating BinaryExpression with null expType in BinaryExpression");
    }
}

void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *BinaryExpression::getLeft() const { return left.get(); }

void BinaryExpression::setLeft(std::unique_ptr<Expression> newLeft) {
    left = std::move(newLeft);
}

BinaryOperator *BinaryExpression::getOperator() const { return op.get(); }

void BinaryExpression::setOperator(std::unique_ptr<BinaryOperator> newOp) {
    op = std::move(newOp);
}

Expression *BinaryExpression::getRight() const { return right.get(); }

void BinaryExpression::setRight(std::unique_ptr<Expression> newRight) {
    right = std::move(newRight);
}

Type *BinaryExpression::getExpType() const { return expType.get(); }

void BinaryExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> left,
                                           std::unique_ptr<Expression> right)
    : left(std::move(left)), right(std::move(right)) {
    if (!this->left) {
        throw std::invalid_argument(
            "Creating AssignmentExpression with null left "
            "in AssignmentExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating AssignmentExpression with null right "
            "in AssignmentExpression");
    }
}

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> left,
                                           std::unique_ptr<Expression> right,
                                           std::unique_ptr<Type> expType)
    : left(std::move(left)), right(std::move(right)),
      expType(std::move(expType)) {
    if (!this->left) {
        throw std::invalid_argument(
            "Creating AssignmentExpression with null left "
            "in AssignmentExpression");
    }
    if (!this->right) {
        throw std::invalid_argument(
            "Creating AssignmentExpression with null right "
            "in AssignmentExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument("Creating AssignmentExpression with null "
                                    "expType in AssignmentExpression");
    }
}

void AssignmentExpression::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *AssignmentExpression::getLeft() const { return left.get(); }

Expression *AssignmentExpression::getRight() const { return right.get(); }

void AssignmentExpression::setLeft(std::unique_ptr<Expression> newLeft) {
    left = std::move(newLeft);
}

void AssignmentExpression::setRight(std::unique_ptr<Expression> newRight) {
    right = std::move(newRight);
}

Type *AssignmentExpression::getExpType() const { return expType.get(); }

void AssignmentExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

ConditionalExpression::ConditionalExpression(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Expression> thenExpression,
    std::unique_ptr<Expression> elseExpression)
    : condition(std::move(condition)),
      thenExpression(std::move(thenExpression)),
      elseExpression(std::move(elseExpression)) {
    if (!this->condition) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "condition in ConditionalExpression");
    }
    if (!this->thenExpression) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "thenExpression in ConditionalExpression");
    }
    if (!this->elseExpression) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "elseExpression in ConditionalExpression");
    }
}

ConditionalExpression::ConditionalExpression(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Expression> thenExpression,
    std::unique_ptr<Expression> elseExpression, std::unique_ptr<Type> expType)
    : condition(std::move(condition)),
      thenExpression(std::move(thenExpression)),
      elseExpression(std::move(elseExpression)), expType(std::move(expType)) {
    if (!this->condition) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "condition in ConditionalExpression");
    }
    if (!this->thenExpression) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "thenExpression in ConditionalExpression");
    }
    if (!this->elseExpression) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "elseExpression in ConditionalExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument("Creating ConditionalExpression with null "
                                    "expType in ConditionalExpression");
    }
}

void ConditionalExpression::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *ConditionalExpression::getCondition() const {
    return condition.get();
}

void ConditionalExpression::setCondition(
    std::unique_ptr<Expression> newCondition) {
    condition = std::move(newCondition);
}

Expression *ConditionalExpression::getThenExpression() const {
    return thenExpression.get();
}

void ConditionalExpression::setThenExpression(
    std::unique_ptr<Expression> newThenExpression) {
    thenExpression = std::move(newThenExpression);
}

Expression *ConditionalExpression::getElseExpression() const {
    return elseExpression.get();
}

void ConditionalExpression::setElseExpression(
    std::unique_ptr<Expression> newElseExpression) {
    elseExpression = std::move(newElseExpression);
}

Type *ConditionalExpression::getExpType() const { return expType.get(); }

void ConditionalExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}

FunctionCallExpression::FunctionCallExpression(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments)
    : identifier(identifier), arguments(std::move(arguments)) {
    if (!this->arguments) {
        throw std::invalid_argument("Creating FunctionCallExpression with null "
                                    "arguments in FunctionCallExpression");
    }
}

FunctionCallExpression::FunctionCallExpression(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments,
    std::unique_ptr<Type> expType)
    : identifier(identifier), arguments(std::move(arguments)),
      expType(std::move(expType)) {
    if (!this->arguments) {
        throw std::invalid_argument("Creating FunctionCallExpression with null "
                                    "arguments in FunctionCallExpression");
    }
    if (!this->expType) {
        throw std::invalid_argument("Creating FunctionCallExpression with null "
                                    "expType in FunctionCallExpression");
    }
}

void FunctionCallExpression::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &FunctionCallExpression::getIdentifier() const {
    return identifier;
}

void FunctionCallExpression::setIdentifier(std::string_view newIdentifier) {
    identifier = newIdentifier;
}

const std::vector<std::unique_ptr<Expression>> &
FunctionCallExpression::getArguments() const {
    return *arguments;
}

void FunctionCallExpression::setArguments(
    std::unique_ptr<std::vector<std::unique_ptr<Expression>>> newArguments) {
    arguments = std::move(newArguments);
}

Type *FunctionCallExpression::getExpType() const { return expType.get(); }

void FunctionCallExpression::setExpType(std::unique_ptr<Type> newExpType) {
    expType = std::move(newExpType);
}
} // Namespace AST
