#include "constant.h"

namespace AST {
ConstantInt::ConstantInt(int value) : value(value) {}

void ConstantInt::accept(Visitor &visitor) { visitor.visit(*this); }

int ConstantInt::getValue() const { return value; }

ConstantLong::ConstantLong(long value) : value(value) {}

void ConstantLong::accept(Visitor &visitor) { visitor.visit(*this); }

long ConstantLong::getValue() const { return value; }
} // Namespace AST
