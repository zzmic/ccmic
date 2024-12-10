#include "operator.h"
#include "visitor.h"
#include <iostream>
#include <sstream>
#include <string>

namespace AST {
void ComplementOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string ComplementOperator::opInString() const { return "~"; }

void NegateOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string NegateOperator::opInString() const { return "-"; }

void AddOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string AddOperator::opInString() const { return "+"; }

void SubtractOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string SubtractOperator::opInString() const { return "-"; }

void MultiplyOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string MultiplyOperator::opInString() const { return "*"; }

void DivideOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string DivideOperator::opInString() const { return "/"; }

void RemainderOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string RemainderOperator::opInString() const { return "%"; }

} // Namespace AST
