#include "declaration.h"
#include "visitor.h"

namespace AST {
VariableDeclaration::VariableDeclaration(const std::string &identifier)
    : identifier(identifier) {}

VariableDeclaration::VariableDeclaration(
    const std::string &identifier,
    std::optional<std::shared_ptr<Expression>> initializer)
    : identifier(identifier), optInitializer(initializer) {}

void VariableDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &VariableDeclaration::getIdentifier() const {
    return identifier;
}

std::optional<std::shared_ptr<Expression>>
VariableDeclaration::getOptInitializer() const {
    return optInitializer;
}

FunctionDeclaration::FunctionDeclaration(
    const std::string &identifier,
    std::shared_ptr<std::vector<std::string>> parameters)
    : identifier(identifier), parameters(parameters) {}

FunctionDeclaration::FunctionDeclaration(
    const std::string &identifier,
    std::shared_ptr<std::vector<std::string>> parameters,
    std::optional<std::shared_ptr<Block>> optBody)
    : identifier(identifier), parameters(parameters), optBody(optBody) {}

void FunctionDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &FunctionDeclaration::getIdentifier() const {
    return identifier;
}

std::shared_ptr<std::vector<std::string>>
FunctionDeclaration::getParameters() const {
    return parameters;
}

std::optional<std::shared_ptr<Block>> FunctionDeclaration::getOptBody() const {
    return optBody;
}

void FunctionDeclaration::setOptBody(
    std::optional<std::shared_ptr<Block>> optBody) {
    this->optBody = optBody;
}
} // Namespace AST
