#include "operator.h"
#include "visitor.h"
#include <iostream>
#include <sstream>
#include <string>

namespace AST {
void ComplementOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string ComplementOperator::opInString() const {
    std::stringstream msg;
    msg << "~";
    return msg.str();
}

void NegateOperator::accept(Visitor &visitor) { visitor.visit(*this); }

std::string NegateOperator::opInString() const {
    std::stringstream msg;
    msg << "-";
    return msg.str();
}
} // Namespace AST
