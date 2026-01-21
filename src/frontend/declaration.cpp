#include "declaration.h"
#include "block.h"
#include "visitor.h"

namespace AST {
VariableDeclaration::VariableDeclaration(std::string_view identifier,
                                         std::unique_ptr<Type> varType)
    : identifier(identifier), varType(std::move(varType)) {}

VariableDeclaration::VariableDeclaration(std::string_view identifier,
                                         std::unique_ptr<Expression> optInit,
                                         std::unique_ptr<Type> varType)
    : identifier(identifier), optInitializer(std::move(optInit)),
      varType(std::move(varType)) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier, std::unique_ptr<Type> varType,
    std::unique_ptr<StorageClass> optStorageClass)
    : identifier(identifier), varType(std::move(varType)),
      optStorageClass(std::move(optStorageClass)) {}

VariableDeclaration::VariableDeclaration(
    std::string_view identifier, std::unique_ptr<Expression> optInit,
    std::unique_ptr<Type> varType,
    std::unique_ptr<StorageClass> optStorageClass)
    : identifier(identifier), optInitializer(std::move(optInit)),
      varType(std::move(varType)), optStorageClass(std::move(optStorageClass)) {
}

void VariableDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &VariableDeclaration::getIdentifier() const {
    return identifier;
}

void VariableDeclaration::setIdentifier(std::string_view newIdentifier) {
    identifier = newIdentifier;
}

Expression *VariableDeclaration::getOptInitializer() const {
    return optInitializer.get();
}

void VariableDeclaration::setOptInitializer(
    std::unique_ptr<Expression> newOptInitializer) {
    optInitializer = std::move(newOptInitializer);
}

Type *VariableDeclaration::getVarType() const { return varType.get(); }

StorageClass *VariableDeclaration::getOptStorageClass() const {
    return optStorageClass.get();
}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<Type> funType)
    : identifier(identifier), parameters(std::move(parameters)),
      funType(std::move(funType)) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<Block> optBody, std::unique_ptr<Type> funType)
    : identifier(identifier), parameters(std::move(parameters)),
      optBody(std::move(optBody)), funType(std::move(funType)) {}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<Type> funType,
    std::unique_ptr<StorageClass> optStorageClass)
    : identifier(identifier), parameters(std::move(parameters)),
      funType(std::move(funType)), optStorageClass(std::move(optStorageClass)) {
}

FunctionDeclaration::FunctionDeclaration(
    std::string_view identifier,
    std::unique_ptr<std::vector<std::string>> parameters,
    std::unique_ptr<Block> optBody, std::unique_ptr<Type> funType,
    std::unique_ptr<StorageClass> optStorageClass)
    : identifier(identifier), parameters(std::move(parameters)),
      optBody(std::move(optBody)), funType(std::move(funType)),
      optStorageClass(std::move(optStorageClass)) {}

// Destructor defined here where Block is complete.
FunctionDeclaration::~FunctionDeclaration() = default;

void FunctionDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &FunctionDeclaration::getIdentifier() const {
    return identifier;
}

const std::vector<std::string> &
FunctionDeclaration::getParameterIdentifiers() const {
    return *parameters;
}

Type *FunctionDeclaration::getFunType() const { return funType.get(); }

Block *FunctionDeclaration::getOptBody() const { return optBody.get(); }

StorageClass *FunctionDeclaration::getOptStorageClass() const {
    return optStorageClass.get();
}

void FunctionDeclaration::setParameters(
    std::unique_ptr<std::vector<std::string>> newParameters) {
    this->parameters = std::move(newParameters);
}

void FunctionDeclaration::setOptBody(std::unique_ptr<Block> newOptBody) {
    this->optBody = std::move(newOptBody);
}

void FunctionDeclaration::setFunType(std::unique_ptr<Type> newFunType) {
    this->funType = std::move(newFunType);
}

void FunctionDeclaration::setOptStorageClass(
    std::unique_ptr<StorageClass> newOptStorageClass) {
    this->optStorageClass = std::move(newOptStorageClass);
}
} // Namespace AST
