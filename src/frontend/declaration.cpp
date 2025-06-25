#include "declaration.h"
#include "visitor.h"

namespace AST {
VariableDeclaration::VariableDeclaration(std::string_view identifier,
                                         std::shared_ptr<Type> varType)
    : identifier(identifier), varType(varType) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier,
    std::optional<std::shared_ptr<Expression>> optInitializer,
    std::shared_ptr<Type> varType)
    : identifier(identifier), optInitializer(optInitializer), varType(varType) {
}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier, std::shared_ptr<Type> varType,
    std::optional<std::shared_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), varType(varType),
      optStorageClass(optStorageClass) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier,
    std::optional<std::shared_ptr<Expression>> optInitializer,
    std::shared_ptr<Type> varType,
    std::optional<std::shared_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), optInitializer(optInitializer), varType(varType),
      optStorageClass(optStorageClass) {}

void VariableDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &VariableDeclaration::getIdentifier() const {
    return identifier;
}

std::optional<std::shared_ptr<Expression>>
VariableDeclaration::getOptInitializer() const {
    return optInitializer;
}

std::shared_ptr<Type> VariableDeclaration::getVarType() const {
    return varType;
}

std::optional<std::shared_ptr<StorageClass>>
VariableDeclaration::getOptStorageClass() const {
    return optStorageClass;
}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::shared_ptr<std::vector<std::string>> parameters,
    std::shared_ptr<Type> funType)
    : identifier(identifier), parameters(parameters), funType(funType) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::shared_ptr<std::vector<std::string>> parameters,
    std::optional<std::shared_ptr<Block>> optBody,
    std::shared_ptr<Type> funType)
    : identifier(identifier), parameters(parameters), optBody(optBody),
      funType(funType) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::shared_ptr<std::vector<std::string>> parameters,
    std::shared_ptr<Type> funType,
    std::optional<std::shared_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), parameters(parameters), funType(funType),
      optStorageClass(optStorageClass) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::shared_ptr<std::vector<std::string>> parameters,
    std::optional<std::shared_ptr<Block>> optBody,
    std::shared_ptr<Type> funType,
    std::optional<std::shared_ptr<StorageClass>> optStorageClass)
    : identifier(identifier), parameters(parameters), optBody(optBody),
      funType(funType), optStorageClass(optStorageClass) {}

void FunctionDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &FunctionDeclaration::getIdentifier() const {
    return identifier;
}

const std::shared_ptr<std::vector<std::string>> &
FunctionDeclaration::getParameterIdentifiers() const {
    return parameters;
}

std::shared_ptr<Type> FunctionDeclaration::getFunType() const {
    return funType;
}

std::optional<std::shared_ptr<Block>> FunctionDeclaration::getOptBody() const {
    return optBody;
}

std::optional<std::shared_ptr<StorageClass>>
FunctionDeclaration::getOptStorageClass() const {
    return optStorageClass;
}

void FunctionDeclaration::setParameters(
    std::shared_ptr<std::vector<std::string>> parameters) {
    this->parameters = parameters;
}

void FunctionDeclaration::setOptBody(
    std::optional<std::shared_ptr<Block>> optBody) {
    this->optBody = optBody;
}

void FunctionDeclaration::setFunType(std::shared_ptr<Type> funType) {
    this->funType = funType;
}

void FunctionDeclaration::setOptStorageClass(
    std::optional<std::shared_ptr<StorageClass>> optStorageClass) {
    this->optStorageClass = optStorageClass;
}
} // Namespace AST
