#include "program.h"
#include "visitor.h"
#include <stdexcept>

namespace AST {
Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>> declarations)
    : declarations(std::move(declarations)) {
    if (!this->declarations) {
        throw std::logic_error(
            "Creating Program with null declarations in Program");
    }
}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

const std::vector<std::unique_ptr<Declaration>> &
Program::getDeclarations() const {
    return *declarations;
}

void Program::setDeclarations(
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>>
        newDeclarations) {
    declarations = std::move(newDeclarations);
}
} // Namespace AST
