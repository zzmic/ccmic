#ifndef FRONTEND_OPERATOR_H
#define FRONTEND_OPERATOR_H

#include "ast.h"
#include <string>
#include <string_view>

namespace AST {
/**
 * Base class for operators in the AST.
 *
 * An operator can be either a unary operator or a binary operator.
 */
class Operator : public AST {};

/**
 * Base class for unary operators in the AST.
 *
 * A unary operator can be complement, negate, or not.
 */
class UnaryOperator : public Operator {};

/**
 * Class representing the complement unary operator.
 */
class ComplementOperator : public UnaryOperator {
  public:
    /**
     * Default constructor for the complement operator.
     */
    constexpr ComplementOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "~"; }
};

/**
 * Class representing the negate unary operator.
 */
class NegateOperator : public UnaryOperator {
  public:
    /**
     * Default constructor for the negate operator.
     */
    constexpr NegateOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "-"; }
};

/**
 * Class representing the not unary operator.
 */
class NotOperator : public UnaryOperator {
  public:
    /**
     * Default constructor for the not operator.
     */
    constexpr NotOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "!"; }
};

/**
 * Base class for binary operators in the AST.
 *
 * A binary operator can be arithmetic, logical, relational, or assignment.
 */
class BinaryOperator : public Operator {};

/**
 * Class representing the addition binary operator.
 */
class AddOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the add operator.
     */
    constexpr AddOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "+"; }
};

/**
 * Class representing the subtraction binary operator.
 */
class SubtractOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the subtract operator.
     */
    constexpr SubtractOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "-"; }
};

/**
 * Class representing the multiplication binary operator.
 */
class MultiplyOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the multiply operator.
     */
    constexpr MultiplyOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "*"; }
};

/**
 * Class representing the division binary operator.
 */
class DivideOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the divide operator.
     */
    constexpr DivideOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "/"; }
};

/**
 * Class representing the remainder binary operator.
 */
class RemainderOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the remainder operator.
     */
    constexpr RemainderOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "%"; }
};

/**
 * Class representing the logical AND binary operator.
 */
class AndOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the logical AND operator.
     */
    constexpr AndOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "&&"; }
};

/**
 * Class representing the logical OR binary operator.
 */
class OrOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the logical OR operator.
     */
    constexpr OrOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "||"; }
};

/**
 * Class representing the equal binary operator.
 */
class EqualOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the equal operator.
     */
    constexpr EqualOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "=="; }
};

/**
 * Class representing the not-equal binary operator.
 */
class NotEqualOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the not-equal operator.
     */
    constexpr NotEqualOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "!="; }
};

/**
 * Class representing the less-than binary operator.
 */
class LessThanOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the less-than operator.
     */
    constexpr LessThanOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "<"; }
};

/**
 * Class representing the less-than-or-equal binary operator.
 */
class LessThanOrEqualOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the less-than-or-equal operator.
     */
    constexpr LessThanOrEqualOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "<="; }
};

/**
 * Class representing the greater-than binary operator.
 */
class GreaterThanOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the greater-than operator.
     */
    constexpr GreaterThanOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return ">"; }
};

/**
 * Class representing the greater-than-or-equal binary operator.
 */
class GreaterThanOrEqualOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the greater-than-or-equal operator.
     */
    constexpr GreaterThanOrEqualOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return ">="; }
};

/**
 * Class representing the assignment binary operator.
 */
class AssignmentOperator : public BinaryOperator {
  public:
    /**
     * Default constructor for the assignment operator.
     */
    constexpr AssignmentOperator() = default;

    void accept(Visitor &visitor) override;

    constexpr std::string_view opInString() const { return "="; }
};
} // Namespace AST

#endif // FRONTEND_OPERATOR_H
