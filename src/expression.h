#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ast.h"
#include "operator.h"
#include <memory>
#include <string>

namespace AST {
class Expression : public AST {};

class Factor : public Expression {};

class ConstantExpression : public Factor {
  public:
    ConstantExpression(int value);
    void accept(Visitor &visitor) override;
    int getValue() const;

  private:
    int value;
};

class VariableExpression : public Factor {
  public:
    VariableExpression(const std::string &identifier);
    void accept(Visitor &visitor) override;
    std::string getIdentifier() const;

  private:
    std::string identifier;
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

} // Namespace AST

#endif // EXPRESSION_H
