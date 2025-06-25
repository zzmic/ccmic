#ifndef CONSTANT_H
#define CONSTANT_H

#include "ast.h"

namespace AST {
class Constant : public AST {};

class ConstantInt : public Constant {
  public:
    constexpr ConstantInt(int value) : value(value) {}
    void accept(Visitor &visitor) override;
    constexpr int getValue() const { return value; }

  private:
    int value;
};

class ConstantLong : public Constant {
  public:
    constexpr ConstantLong(long value) : value(value) {}
    void accept(Visitor &visitor) override;
    constexpr long getValue() const { return value; }

  private:
    long value;
};
} // Namespace AST

#endif // CONSTANT_H
