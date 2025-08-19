#include "function.h"
#include "visitor.h"

namespace AST {
Function::Function(std::string_view identifier, Block *body)
    : identifier(identifier), body(body) {}

Function::~Function() {
    delete body; // Clean up raw pointer.
}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &Function::getIdentifier() { return identifier; }

Block *Function::getBody() { return body; }

void Function::setBody(Block *newBody) { this->body = newBody; }
} // Namespace AST
