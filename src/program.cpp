

#include "program.h"
#include "visitor.h"

namespace AST {
Program::Program(std::shared_ptr<Function> function) : function_(function) {}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Function> Program::getFunction() const { return function_; }
} // Namespace AST
