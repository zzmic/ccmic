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
} // Namespace AST

#endif // OPERATOR_H
