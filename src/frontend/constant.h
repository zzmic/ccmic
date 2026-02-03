#ifndef CONSTANT_H
#define CONSTANT_H

#include "ast.h"
#include "visitor.h"

namespace AST {
/**
 * Base class for constants in the AST.
 *
 * A constant can be either an integer constant or a long constant.
 */
class Constant : public AST {};

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
    constexpr explicit ConstantInt(int value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr int getValue() const { return value; }

  private:
    /**
     * The integer value/representation of the constant.
     */
    int value;
};

/**
 * Class representing a long constant.
 */
class ConstantLong : public Constant {
  public:
    /**
     * Constructor for the constant long class.
     *
     * @param value The long value of the constant.
     */
    constexpr explicit ConstantLong(long value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr long getValue() const { return value; }

  private:
    /**
     * The long value/representation of the constant.
     */
    long value;
};

/**
 * Class representing an unsigned integer constant.
 */
class ConstantUInt : public Constant {
  public:
    /**
     * Constructor for the constant unsigned integer class.
     *
     * @param value The unsigned integer value of the constant.
     */
    constexpr explicit ConstantUInt(unsigned int value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr unsigned int getValue() const { return value; }

  private:
    /**
     * The unsigned integer value/representation of the constant.
     */
    unsigned int value;
};

class ConstantULong : public Constant {
  public:
    /**
     * Constructor for the constant unsigned long class.
     *
     * @param value The unsigned long value of the constant.
     */
    constexpr explicit ConstantULong(unsigned long value) : value(value) {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] constexpr unsigned long getValue() const { return value; }

  private:
    /**
     * The unsigned long value/representation of the constant.
     */
    unsigned long value;
};
} // Namespace AST

#endif // CONSTANT_H
