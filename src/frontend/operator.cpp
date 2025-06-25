#include "operator.h"
#include "visitor.h"

namespace AST {
void ComplementOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void NegateOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void NotOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void AddOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void SubtractOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void MultiplyOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void DivideOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void RemainderOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void AndOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void OrOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void EqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void NotEqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void LessThanOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void LessThanOrEqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void GreaterThanOperator::accept(Visitor &visitor) { visitor.visit(*this); }

void GreaterThanOrEqualOperator::accept(Visitor &visitor) {
    visitor.visit(*this);
}

void AssignmentOperator::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
