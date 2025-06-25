#ifndef FRONTEND_OPERATOR_H
#define FRONTEND_OPERATOR_H

#include "ast.h"
#include <string>
#include <string_view>

namespace AST {
class Operator : public AST {};

class UnaryOperator : public Operator {};

class ComplementOperator : public UnaryOperator {
  public:
    constexpr ComplementOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "~"; }
};

class NegateOperator : public UnaryOperator {
  public:
    constexpr NegateOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "-"; }
};

class NotOperator : public UnaryOperator {
  public:
    constexpr NotOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "!"; }
};

class BinaryOperator : public Operator {};

class AddOperator : public BinaryOperator {
  public:
    constexpr AddOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "+"; }
};

class SubtractOperator : public BinaryOperator {
  public:
    constexpr SubtractOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "-"; }
};

class MultiplyOperator : public BinaryOperator {
  public:
    constexpr MultiplyOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "*"; }
};

class DivideOperator : public BinaryOperator {
  public:
    constexpr DivideOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "/"; }
};

class RemainderOperator : public BinaryOperator {
  public:
    constexpr RemainderOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "%"; }
};

class AndOperator : public BinaryOperator {
  public:
    constexpr AndOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "&&"; }
};

class OrOperator : public BinaryOperator {
  public:
    constexpr OrOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "||"; }
};

class EqualOperator : public BinaryOperator {
  public:
    constexpr EqualOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "=="; }
};

class NotEqualOperator : public BinaryOperator {
  public:
    constexpr NotEqualOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "!="; }
};

class LessThanOperator : public BinaryOperator {
  public:
    constexpr LessThanOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "<"; }
};

class LessThanOrEqualOperator : public BinaryOperator {
  public:
    constexpr LessThanOrEqualOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "<="; }
};

class GreaterThanOperator : public BinaryOperator {
  public:
    constexpr GreaterThanOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return ">"; }
};

class GreaterThanOrEqualOperator : public BinaryOperator {
  public:
    constexpr GreaterThanOrEqualOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return ">="; }
};

class AssignmentOperator : public BinaryOperator {
  public:
    constexpr AssignmentOperator() = default;
    void accept(Visitor &visitor) override;
    constexpr std::string_view opInString() const { return "="; }
};
} // Namespace AST

#endif // FRONTEND_OPERATOR_H
