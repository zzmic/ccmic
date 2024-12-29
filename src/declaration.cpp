#include "declaration.h"
#include "visitor.h"

namespace AST {
Declaration::Declaration(const std::string &identifier)
    : identifier(identifier) {}

Declaration::Declaration(const std::string &identifier,
                         std::optional<std::shared_ptr<Expression>> initializer)
    : identifier(identifier), optInitializer(initializer) {}

void Declaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &Declaration::getIdentifier() const { return identifier; }

std::optional<std::shared_ptr<Expression>>
Declaration::getOptInitializer() const {
    return optInitializer;
}
} // Namespace AST
