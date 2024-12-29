#include "function.h"
#include "visitor.h"

namespace AST {
Function::Function(
    const std::string &name,
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> body)
    : name(name), body(body) {}

void Function::accept(Visitor &visitor) { visitor.visit(*this); }

std::string Function::getName() const { return name; }

std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>>
Function::getBody() const {
    return body;
}
} // Namespace AST
