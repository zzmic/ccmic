#include "function.h"
#include "visitor.h"

namespace AST {
Function::Function(std::string_view identifier, std::shared_ptr<Block> body)
    : identifier(identifier), body(body) {}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &Function::getIdentifier() const { return identifier; }

std::shared_ptr<Block> Function::getBody() const { return body; }

void Function::setBody(std::shared_ptr<Block> newBody) { this->body = newBody; }
} // Namespace AST
