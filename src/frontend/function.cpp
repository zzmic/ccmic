#include "function.h"
#include "visitor.h"

namespace AST {
Function::Function(const std::string &name, std::shared_ptr<Block> body)
    : name(name), body(body) {}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

std::string Function::getName() const { return name; }

std::shared_ptr<Block> Function::getBody() const { return body; }

void Function::setBody(std::shared_ptr<Block> body) { this->body = body; }
} // Namespace AST
