#include "program.h"
#include "visitor.h"

namespace AST {
Program::Program(
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
        functionDeclarations)
    : functionDeclarations(functionDeclarations) {}

void Program::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<std::vector<std::shared_ptr<FunctionDeclaration>>>
Program::getFunctionDeclarations() const {
    return functionDeclarations;
}
} // Namespace AST
