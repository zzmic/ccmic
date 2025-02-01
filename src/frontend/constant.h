#ifndef CONSTANT_H
#define CONSTANT_H

#include "ast.h"

namespace AST {
class Constant : public AST {};

class ConstantInt : public Constant {
  public:
    ConstantInt(int value);
    void accept(Visitor &visitor) override;
    int getValue() const;

  private:
    int value;
};

class ConstantLong : public Constant {
  public:
    ConstantLong(long value);
    void accept(Visitor &visitor) override;
    long getValue() const;

  private:
    long value;
};
} // Namespace AST

#endif // CONSTANT_H
