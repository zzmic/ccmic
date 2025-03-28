#ifndef FRONTEND_EXPRESSION_H
#define FRONTEND_EXPRESSION_H

#include "ast.h"
#include "constant.h"
#include "operator.h"
#include "type.h"
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace AST {
class Expression : public AST {
  public:
    virtual std::shared_ptr<Type> getExpType() const = 0;
    virtual void setExpType(std::shared_ptr<Type> expType) = 0;
};

class Factor : public Expression {};

class ConstantExpression : public Factor {
  public:
    ConstantExpression(std::shared_ptr<Constant> constant);
    ConstantExpression(std::shared_ptr<Constant> constant,
                       std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Constant> getConstant() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;
    int getConstantInInt() const;
    std::variant<int, long> getConstantInIntOrLongVariant() const;

  private:
    std::shared_ptr<Constant> constant;
    // Type information of, in this case, the constant-expression AST node.
    std::shared_ptr<Type> expType;
};

class VariableExpression : public Factor {
  public:
    VariableExpression(const std::string &identifier);
    VariableExpression(const std::string &identifier,
                       std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::string getIdentifier() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::shared_ptr<Type> expType;
};

class CastExpression : public Factor {
  public:
    CastExpression(std::shared_ptr<Type> targetType,
                   std::shared_ptr<Expression> expr);
    CastExpression(std::shared_ptr<Type> targetType,
                   std::shared_ptr<Expression> expr,
                   std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Type> getTargetType() const;
    std::shared_ptr<Expression> getExpression() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Type> targetType;
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Type> expType;
};

class UnaryExpression : public Factor {
  public:
    UnaryExpression(const std::string &opInStr,
                    std::shared_ptr<Expression> expr);
    UnaryExpression(const std::string &opInStr,
                    std::shared_ptr<Expression> expr,
                    std::shared_ptr<Type> expType);
    UnaryExpression(std::shared_ptr<UnaryOperator> op,
                    std::shared_ptr<Factor> expr);
    UnaryExpression(std::shared_ptr<UnaryOperator> op,
                    std::shared_ptr<Factor> expr,
                    std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<UnaryOperator> getOperator() const;
    std::shared_ptr<Factor> getExpression() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<UnaryOperator> op;
    std::shared_ptr<Factor> expr;
    std::shared_ptr<Type> expType;
};

class BinaryExpression : public Expression {
  public:
    BinaryExpression(std::shared_ptr<Expression> left,
                     const std::string &opInStr,
                     std::shared_ptr<Expression> right);
    BinaryExpression(std::shared_ptr<Expression> left,
                     const std::string &opInStr,
                     std::shared_ptr<Expression> right,
                     std::shared_ptr<Type> expType);
    BinaryExpression(std::shared_ptr<Expression> left,
                     std::shared_ptr<BinaryOperator> op,
                     std::shared_ptr<Expression> right);
    BinaryExpression(std::shared_ptr<Expression> left,
                     std::shared_ptr<BinaryOperator> op,
                     std::shared_ptr<Expression> right,
                     std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getLeft() const;
    std::shared_ptr<BinaryOperator> getOperator() const;
    std::shared_ptr<Expression> getRight() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<BinaryOperator> op;
    std::shared_ptr<Expression> right;
    std::shared_ptr<Type> expType;
};

class AssignmentExpression : public Expression {
  public:
    AssignmentExpression(std::shared_ptr<Expression> left,
                         std::shared_ptr<Expression> right);
    AssignmentExpression(std::shared_ptr<Expression> left,
                         std::shared_ptr<Expression> right,
                         std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getLeft() const;
    std::shared_ptr<Expression> getRight() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    std::shared_ptr<Type> expType;
};

class ConditionalExpression : public Expression {
  public:
    ConditionalExpression(std::shared_ptr<Expression> condition,
                          std::shared_ptr<Expression> thenExpression,
                          std::shared_ptr<Expression> elseExpression);
    ConditionalExpression(std::shared_ptr<Expression> condition,
                          std::shared_ptr<Expression> thenExpression,
                          std::shared_ptr<Expression> elseExpression,
                          std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Expression> getThenExpression() const;
    std::shared_ptr<Expression> getElseExpression() const;
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> thenExpression;
    std::shared_ptr<Expression> elseExpression;
    std::shared_ptr<Type> expType;
};

class FunctionCallExpression : public Expression {
  public:
    FunctionCallExpression(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);
    FunctionCallExpression(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments,
        std::shared_ptr<Type> expType);
    void accept(Visitor &visitor) override;
    std::string getIdentifier() const;
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>>
    getArguments() const;
    void setArguments(
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);
    std::shared_ptr<Type> getExpType() const override;
    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments;
    std::shared_ptr<Type> expType;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
