#include "function.h"
#include "visitor.h"
#include <stdexcept>

namespace AST {
Function::Function(std::string_view identifier, std::unique_ptr<Block> body)
    : identifier(identifier), body(std::move(body)) {
    if (!this->body) {
        throw std::logic_error("Creating Function with null body");
    }
}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &Function::getIdentifier() const { return identifier; }

Block *Function::getBody() const { return body.get(); }

void Function::setBody(std::unique_ptr<Block> newBody) {
    body = std::move(newBody);
}
} // Namespace AST
