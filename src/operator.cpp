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

void NotOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string NotOperator::opInString() const { return "!"; }

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

void AndOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string AndOperator::opInString() const { return "&&"; }

void OrOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string OrOperator::opInString() const { return "||"; }

void EqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string EqualOperator::opInString() const { return "=="; }

void NotEqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string NotEqualOperator::opInString() const { return "!="; }

void LessThanOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string LessThanOperator::opInString() const { return "<"; }

void LessThanOrEqualOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string LessThanOrEqualOperator::opInString() const { return "<="; }

void GreaterThanOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string GreaterThanOperator::opInString() const { return ">"; }

void GreaterThanOrEqualOperator::accept(Visitor &visitor) {
    visitor.visit(*this);
}

std::string GreaterThanOrEqualOperator::opInString() const { return ">="; }

void AssignmentOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string AssignmentOperator::opInString() const { return "="; }
} // Namespace AST
