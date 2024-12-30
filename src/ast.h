#ifndef AST_H
#define AST_H

#include "visitor.h"

namespace AST {
class AST {
  public:
    // `= default` instructs the compiler to generate the default
    // implementation.
    virtual ~AST() = default;
    // Virtual accept function to visit the AST node.
    virtual void accept(Visitor &visitor) = 0;
};
} // Namespace AST

#endif // AST_H
