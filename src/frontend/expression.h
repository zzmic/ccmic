#ifndef FRONTEND_EXPRESSION_H
#define FRONTEND_EXPRESSION_H

#include "ast.h"
#include "constant.h"
#include "operator.h"
#include "type.h"
#include <memory>
#include <string>
#include <vector>

namespace AST {
class Expression : public AST {};

class Factor : public Expression {};

class ConstantExpression : public Factor {
  public:
    ConstantExpression(std::shared_ptr<Constant> constant);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Constant> getConstant() const;
    // TODO(zzmic): This is a temporary solution.
    int getConstantInInt() const;

  private:
    std::shared_ptr<Constant> constant;
};

class VariableExpression : public Factor {
  public:
    VariableExpression(const std::string &identifier);
    void accept(Visitor &visitor) override;
    std::string getIdentifier() const;

  private:
    std::string identifier;
};

class CastExpression : public Factor {
  public:
    CastExpression(std::shared_ptr<Type> targetType,
                   std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Type> getTargetType() const;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Type> targetType;
    std::shared_ptr<Expression> expr;
};

class UnaryExpression : public Factor {
  public:
    UnaryExpression(const std::string &opInStr,
                    std::shared_ptr<Expression> expr);
    UnaryExpression(std::shared_ptr<UnaryOperator> op,
                    std::shared_ptr<Factor> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<UnaryOperator> getOperator() const;
    std::shared_ptr<Factor> getExpression() const;

  private:
    std::shared_ptr<UnaryOperator> op;
    std::shared_ptr<Factor> expr;
};

class BinaryExpression : public Expression {
  public:
    BinaryExpression(std::shared_ptr<Expression> left,
                     const std::string &opInStr,
                     std::shared_ptr<Expression> right);
    BinaryExpression(std::shared_ptr<Expression> left,
                     std::shared_ptr<BinaryOperator> op,
                     std::shared_ptr<Expression> right);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getLeft() const;
    std::shared_ptr<BinaryOperator> getOperator() const;
    std::shared_ptr<Expression> getRight() const;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<BinaryOperator> op;
    std::shared_ptr<Expression> right;
};

class AssignmentExpression : public Expression {
  public:
    AssignmentExpression(std::shared_ptr<Expression> left,
                         std::shared_ptr<Expression> right);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getLeft() const;
    std::shared_ptr<Expression> getRight() const;

  private:
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
};

class ConditionalExpression : public Expression {
  public:
    ConditionalExpression(std::shared_ptr<Expression> condition,
                          std::shared_ptr<Expression> thenExpression,
                          std::shared_ptr<Expression> elseExpression);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Expression> getThenExpression() const;
    std::shared_ptr<Expression> getElseExpression() const;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> thenExpression;
    std::shared_ptr<Expression> elseExpression;
};

class FunctionCallExpression : public Expression {
  public:
    FunctionCallExpression(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);
    void accept(Visitor &visitor) override;
    std::string getIdentifier() const;
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>>
    getArguments() const;

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
