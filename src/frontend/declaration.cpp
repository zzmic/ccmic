#include "declaration.h"
#include "block.h"
#include "visitor.h"

namespace AST {
VariableDeclaration::VariableDeclaration(std::string_view identifier,
                                         std::unique_ptr<Type> varType)
    : identifier(identifier), varType(std::move(varType)) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier,
    std::optional<std::unique_ptr<Expression>> optInitializer,
    std::unique_ptr<Type> varType)
    : identifier(identifier), optInitializer(std::move(optInitializer)),
      varType(std::move(varType)) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier, std::unique_ptr<Type> varType,
    std::optional<std::unique_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), varType(std::move(varType)),
      optStorageClass(std::move(optStorageClass)) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier,
    std::optional<std::unique_ptr<Expression>> optInitializer,
    std::unique_ptr<Type> varType,
    std::optional<std::unique_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), optInitializer(std::move(optInitializer)),
      varType(std::move(varType)), optStorageClass(std::move(optStorageClass)) {
}

void VariableDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &VariableDeclaration::getIdentifier() { return identifier; }

std::optional<std::unique_ptr<Expression>> &
VariableDeclaration::getOptInitializer() {
    return optInitializer;
}

std::unique_ptr<Type> &VariableDeclaration::getVarType() { return varType; }

std::optional<std::unique_ptr<StorageClass>> &
VariableDeclaration::getOptStorageClass() {
    return optStorageClass;
}

FunctionDeclaration::FunctionDeclaration(std::string_view identifier,
                                         std::vector<std::string> parameters,
                                         std::unique_ptr<Type> funType)
    : identifier(identifier), parameters(std::move(parameters)),
      funType(std::move(funType)) {}

FunctionDeclaration::FunctionDeclaration(std::string_view identifier,
                                         std::vector<std::string> parameters,
                                         std::optional<Block *> optBody,
                                         std::unique_ptr<Type> funType)
    : identifier(identifier), parameters(std::move(parameters)),
      optBody(optBody), funType(std::move(funType)) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier, std::vector<std::string> parameters,
    std::unique_ptr<Type> funType,
    std::optional<std::unique_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), parameters(std::move(parameters)),
      funType(std::move(funType)), optStorageClass(std::move(optStorageClass)) {
}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier, std::vector<std::string> parameters,
    std::optional<Block *> optBody, std::unique_ptr<Type> funType,
    std::optional<std::unique_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), parameters(std::move(parameters)),
      optBody(optBody), funType(std::move(funType)),
      optStorageClass(std::move(optStorageClass)) {}

void FunctionDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &FunctionDeclaration::getIdentifier() { return identifier; }

std::vector<std::string> &FunctionDeclaration::getParameterIdentifiers() {
    return parameters;
}

std::unique_ptr<Type> &FunctionDeclaration::getFunType() { return funType; }

std::optional<Block *> &FunctionDeclaration::getOptBody() { return optBody; }

std::optional<std::unique_ptr<StorageClass>> &
FunctionDeclaration::getOptStorageClass() {
    return optStorageClass;
}

void FunctionDeclaration::setParameters(
    std::vector<std::string> newParameters) {
    this->parameters = std::move(newParameters);
}

void FunctionDeclaration::setOptBody(std::optional<Block *> newOptBody) {
    this->optBody = newOptBody;
}

FunctionDeclaration::~FunctionDeclaration() {
    // Clean up the raw pointer if it exists.
    if (optBody.has_value()) {
        delete optBody.value();
    }
}

void FunctionDeclaration::setFunType(std::unique_ptr<Type> newFunType) {
    this->funType = std::move(newFunType);
}

void FunctionDeclaration::setOptStorageClass(
    std::optional<std::unique_ptr<StorageClass>> newOptStorageClass) {
    this->optStorageClass = std::move(newOptStorageClass);
}
} // Namespace AST
