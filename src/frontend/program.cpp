#include "program.h"
#include "visitor.h"

namespace AST {
Program::Program(
    std::shared_ptr<std::vector<std::shared_ptr<Declaration>>> declarations)
    : declarations(declarations) {}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

const std::shared_ptr<std::vector<std::shared_ptr<Declaration>>> &
Program::getDeclarations() const {
    return declarations;
}

void Program::setDeclarations(
    std::shared_ptr<std::vector<std::shared_ptr<Declaration>>> declarations) {
    this->declarations = declarations;
}
} // Namespace AST
