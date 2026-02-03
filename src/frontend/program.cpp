#include "program.h"
#include "visitor.h"
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace AST {
Program::Program(
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>> declarations)
    : declarations(std::move(declarations)) {
    if (!this->declarations) {
        throw std::invalid_argument(
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
