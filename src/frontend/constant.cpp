#include "constant.h"

namespace AST {
void ConstantInt::accept(Visitor &visitor) { visitor.visit(*this); }

void ConstantLong::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
