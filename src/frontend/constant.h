#ifndef CONSTANT_H
#define CONSTANT_H

#include "ast.h"

namespace AST {
/**
 * Base class for constants in the AST.
 *
 * A constant can be either an integer constant or a long constant.
 */
class Constant : public AST {
  public:
    /**
     * Default constructor for the constant class.
     */
    constexpr Constant() = default;
};

/**
 * Class representing an integer constant.
 */
class ConstantInt : public Constant {
  public:
    /**
     * Constructor for the constant integer class.
     *
     * @param value The integer value of the constant.
     */
    constexpr ConstantInt(int value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr int getValue() const { return value; }

  private:
    /**
     * The integer value/representation of the constant.
     */
    int value;
};

class ConstantLong : public Constant {
  public:
    /**
     * Constructor for the constant long class.
     *
     * @param value The long value of the constant.
     */
    constexpr ConstantLong(long value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr long getValue() const { return value; }

  private:
    /**
     * The long value/representation of the constant.
     */
    long value;
};
} // Namespace AST

#endif // CONSTANT_H
