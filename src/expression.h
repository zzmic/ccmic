

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "ast.h"
#include "operator.h"
#include <memory>
#include <string>

namespace AST {
class Expression : public AST {};

class ConstantExpression : public Expression {
  public:
    ConstantExpression(int value);
    void accept(Visitor &visitor) override;
    int getValue() const;

  private:
    int value_;
};

class UnaryExpression : public Expression {
  public:
    UnaryExpression(const std::string &op, std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<UnaryOperator> getOperator() const;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<UnaryOperator> op_;
    std::shared_ptr<Expression> expr_;
};

class BinaryExpression : public Expression {
  public:
    BinaryExpression(std::shared_ptr<Expression> left, const std::string &op,
                     std::shared_ptr<Expression> right);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getLeft() const;
    std::string getOperator() const;
    std::shared_ptr<Expression> getRight() const;

  private:
    std::shared_ptr<Expression> left_;
    std::string op_;
    std::shared_ptr<Expression> right_;
};
} // Namespace AST

#endif // EXPRESSION_H
