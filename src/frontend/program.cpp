#include "program.h"
#include "visitor.h"

namespace AST {
Program::Program(std::vector<std::unique_ptr<Declaration>> declarations)
    : declarations(std::move(declarations)) {}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

std::vector<std::unique_ptr<Declaration>> &Program::getDeclarations() {
    return declarations;
}

void Program::setDeclarations(
    std::vector<std::unique_ptr<Declaration>> newDeclarations) {
    this->declarations = std::move(newDeclarations);
}
} // Namespace AST
