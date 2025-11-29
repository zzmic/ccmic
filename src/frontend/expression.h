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
    virtual std::shared_ptr<Type> getExpType() const = 0;
    virtual void setExpType(std::shared_ptr<Type> expType) = 0;
};

class Factor : public Expression {};

class ConstantExpression : public Factor {
  public:
    explicit ConstantExpression(std::shared_ptr<Constant> constant);
    explicit ConstantExpression(std::shared_ptr<Constant> constant,
                                std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Constant> getConstant() const;
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;
    [[nodiscard]] int getConstantInInt() const;
    [[nodiscard]] std::variant<int, long> getConstantInIntOrLongVariant() const;

  private:
    std::shared_ptr<Constant> constant;
    // Type information of, in this case, the constant-expression AST node.
    std::shared_ptr<Type> expType;
};

class VariableExpression : public Factor {
  public:
    explicit VariableExpression(std::string_view identifier);
    explicit VariableExpression(std::string_view identifier,
                                std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getIdentifier() const;
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::shared_ptr<Type> expType;
};

class CastExpression : public Factor {
  public:
    explicit CastExpression(std::shared_ptr<Type> targetType,
                            std::shared_ptr<Expression> expr);
    explicit CastExpression(std::shared_ptr<Type> targetType,
                            std::shared_ptr<Expression> expr,
                            std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Type> getTargetType() const;
    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Type> targetType;
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Type> expType;
};

class UnaryExpression : public Factor {
  public:
    explicit UnaryExpression(std::string_view opInStr,
                             std::shared_ptr<Expression> expr);
    explicit UnaryExpression(std::string_view opInStr,
                             std::shared_ptr<Expression> expr,
                             std::shared_ptr<Type> expType);
    explicit UnaryExpression(std::shared_ptr<UnaryOperator> op,
                             std::shared_ptr<Factor> expr);
    explicit UnaryExpression(std::shared_ptr<UnaryOperator> op,
                             std::shared_ptr<Factor> expr,
                             std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<UnaryOperator> getOperator() const;
    [[nodiscard]] std::shared_ptr<Factor> getExpression() const;
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<UnaryOperator> op;
    std::shared_ptr<Factor> expr;
    std::shared_ptr<Type> expType;
};

class BinaryExpression : public Expression {
  public:
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::string_view opInStr,
                              std::shared_ptr<Expression> right);
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::string_view opInStr,
                              std::shared_ptr<Expression> right,
                              std::shared_ptr<Type> expType);
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::shared_ptr<BinaryOperator> op,
                              std::shared_ptr<Expression> right);
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::shared_ptr<BinaryOperator> op,
                              std::shared_ptr<Expression> right,
                              std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getLeft() const;
    void setLeft(std::shared_ptr<Expression> left);
    [[nodiscard]] std::shared_ptr<BinaryOperator> getOperator() const;
    void setOperator(std::shared_ptr<BinaryOperator> op);
    [[nodiscard]] std::shared_ptr<Expression> getRight() const;
    void setRight(std::shared_ptr<Expression> right);
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<BinaryOperator> op;
    std::shared_ptr<Expression> right;
    std::shared_ptr<Type> expType;
};

class AssignmentExpression : public Expression {
  public:
    explicit AssignmentExpression(std::shared_ptr<Expression> left,
                                  std::shared_ptr<Expression> right);
    explicit AssignmentExpression(std::shared_ptr<Expression> left,
                                  std::shared_ptr<Expression> right,
                                  std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getLeft() const;
    [[nodiscard]] std::shared_ptr<Expression> getRight() const;
    void setLeft(std::shared_ptr<Expression> left);
    void setRight(std::shared_ptr<Expression> right);
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    std::shared_ptr<Type> expType;
};

class ConditionalExpression : public Expression {
  public:
    explicit ConditionalExpression(std::shared_ptr<Expression> condition,
                                   std::shared_ptr<Expression> thenExpression,
                                   std::shared_ptr<Expression> elseExpression);
    explicit ConditionalExpression(std::shared_ptr<Expression> condition,
                                   std::shared_ptr<Expression> thenExpression,
                                   std::shared_ptr<Expression> elseExpression,
                                   std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;
    void setCondition(std::shared_ptr<Expression> condition);
    [[nodiscard]] std::shared_ptr<Expression> getThenExpression() const;
    void setThenExpression(std::shared_ptr<Expression> thenExpression);
    [[nodiscard]] std::shared_ptr<Expression> getElseExpression() const;
    void setElseExpression(std::shared_ptr<Expression> elseExpression);
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> thenExpression;
    std::shared_ptr<Expression> elseExpression;
    std::shared_ptr<Type> expType;
};

class FunctionCallExpression : public Expression {
  public:
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments,
        std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getIdentifier() const;
    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<Expression>>> &
    getArguments() const;
    void setArguments(
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);
    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments;
    std::shared_ptr<Type> expType;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
