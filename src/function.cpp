#include "function.h"
#include "visitor.h"

namespace AST {
Function::Function(const std::string &name, std::shared_ptr<Statement> body)
    : name_(name), body_(body) {}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

std::string Function::getName() const { return name_; }

std::shared_ptr<Statement> Function::getBody() const { return body_; }
} // Namespace AST
