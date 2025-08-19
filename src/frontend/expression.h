#ifndef FRONTEND_EXPRESSION_H
#define FRONTEND_EXPRESSION_H

#include "ast.h"
#include "constant.h"
#include "operator.h"
#include "type.h"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace AST {
class Expression : public AST {
  public:
    constexpr Expression() = default;
    virtual std::unique_ptr<Type> &getExpType() = 0;
    virtual void setExpType(std::unique_ptr<Type> expType) = 0;
};

class Factor : public Expression {};

class ConstantExpression : public Factor {
  public:
    explicit ConstantExpression(std::unique_ptr<Constant> constant);
    explicit ConstantExpression(std::unique_ptr<Constant> constant,
                                std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Constant> &getConstant();
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;
    [[nodiscard]] int getConstantInInt() const;
    [[nodiscard]] std::variant<int, long> getConstantInIntOrLongVariant() const;

  private:
    std::unique_ptr<Constant> constant;
    // Type information of, in this case, the constant-expression AST node.
    std::unique_ptr<Type> expType;
};

class VariableExpression : public Factor {
  public:
    explicit VariableExpression(std::string_view identifier);
    explicit VariableExpression(std::string_view identifier,
                                std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getIdentifier();
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::unique_ptr<Type> expType;
};

class CastExpression : public Factor {
  public:
    explicit CastExpression(std::unique_ptr<Type> targetType,
                            std::unique_ptr<Expression> expr);
    explicit CastExpression(std::unique_ptr<Type> targetType,
                            std::unique_ptr<Expression> expr,
                            std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Type> &getTargetType();
    [[nodiscard]] std::unique_ptr<Expression> &getExpression();
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::unique_ptr<Type> targetType;
    std::unique_ptr<Expression> expr;
    std::unique_ptr<Type> expType;
};

class UnaryExpression : public Factor {
  public:
    explicit UnaryExpression(std::string_view opInStr,
                             std::unique_ptr<Expression> expr);
    explicit UnaryExpression(std::string_view opInStr,
                             std::unique_ptr<Expression> expr,
                             std::unique_ptr<Type> expType);
    explicit UnaryExpression(std::unique_ptr<UnaryOperator> op,
                             std::unique_ptr<Factor> expr);
    explicit UnaryExpression(std::unique_ptr<UnaryOperator> op,
                             std::unique_ptr<Factor> expr,
                             std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<UnaryOperator> &getOperator();
    [[nodiscard]] std::unique_ptr<Factor> &getExpression();
    void setExpression(std::unique_ptr<Factor> newExpr);
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::unique_ptr<UnaryOperator> op;
    std::unique_ptr<Factor> expr;
    std::unique_ptr<Type> expType;
};

class BinaryExpression : public Expression {
  public:
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::string_view opInStr,
                              std::unique_ptr<Expression> right);
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::string_view opInStr,
                              std::unique_ptr<Expression> right,
                              std::unique_ptr<Type> expType);
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::unique_ptr<BinaryOperator> op,
                              std::unique_ptr<Expression> right);
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::unique_ptr<BinaryOperator> op,
                              std::unique_ptr<Expression> right,
                              std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getLeft();
    void setLeft(std::unique_ptr<Expression> newLeft);
    [[nodiscard]] std::unique_ptr<BinaryOperator> &getOperator();
    void setOperator(std::unique_ptr<BinaryOperator> newOp);
    [[nodiscard]] std::unique_ptr<Expression> &getRight();
    void setRight(std::unique_ptr<Expression> newRight);
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<BinaryOperator> op;
    std::unique_ptr<Expression> right;
    std::unique_ptr<Type> expType;
};

class AssignmentExpression : public Expression {
  public:
    explicit AssignmentExpression(std::unique_ptr<Expression> left,
                                  std::unique_ptr<Expression> right);
    explicit AssignmentExpression(std::unique_ptr<Expression> left,
                                  std::unique_ptr<Expression> right,
                                  std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getLeft();
    void setLeft(std::unique_ptr<Expression> newLeft);
    [[nodiscard]] std::unique_ptr<Expression> &getRight();
    void setRight(std::unique_ptr<Expression> newRight);
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    std::unique_ptr<Type> expType;
};

class ConditionalExpression : public Expression {
  public:
    explicit ConditionalExpression(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Expression> thenExpression,
                                   std::unique_ptr<Expression> elseExpression);
    explicit ConditionalExpression(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Expression> thenExpression,
                                   std::unique_ptr<Expression> elseExpression,
                                   std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getCondition();
    void setCondition(std::unique_ptr<Expression> newCondition);
    [[nodiscard]] std::unique_ptr<Expression> &getThenExpression();
    void setThenExpression(std::unique_ptr<Expression> newThenExpression);
    [[nodiscard]] std::unique_ptr<Expression> &getElseExpression();
    void setElseExpression(std::unique_ptr<Expression> newElseExpression);
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> thenExpression;
    std::unique_ptr<Expression> elseExpression;
    std::unique_ptr<Type> expType;
};

class FunctionCallExpression : public Expression {
  public:
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::vector<std::unique_ptr<Expression>> arguments);
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::vector<std::unique_ptr<Expression>> arguments,
        std::unique_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getIdentifier();
    [[nodiscard]] std::vector<std::unique_ptr<Expression>> &getArguments();
    void setArguments(std::vector<std::unique_ptr<Expression>> newArguments);
    [[nodiscard]] std::unique_ptr<Type> &getExpType() override;
    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::vector<std::unique_ptr<Expression>> arguments;
    std::unique_ptr<Type> expType;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
