#include "program.h"
#include "visitor.h"

namespace AST {
Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>> declarations)
    : declarations(std::move(declarations)) {}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

const std::vector<std::unique_ptr<Declaration>> &
Program::getDeclarations() const {
    return *declarations;
}

void Program::setDeclarations(
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>>
        newDeclarations) {
    this->declarations = std::move(newDeclarations);
}
} // Namespace AST
