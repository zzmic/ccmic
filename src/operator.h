#ifndef OPERATOR_H
#define OPERATOR_H

#include "ast.h"
#include <string>

namespace AST {
class Operator : public AST {};

class UnaryOperator : public Operator {};

class ComplementOperator : public UnaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class NegateOperator : public UnaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class NotOperator : public UnaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class BinaryOperator : public Operator {};

class AddOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class SubtractOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class MultiplyOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class DivideOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class RemainderOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class AndOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class OrOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class EqualOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class NotEqualOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class LessThanOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class LessThanOrEqualOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class GreaterThanOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class GreaterThanOrEqualOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};

class AssignmentOperator : public BinaryOperator {
  public:
    void accept(Visitor &visitor) override;
    std::string opInString() const;
};
} // Namespace AST

#endif // OPERATOR_H
