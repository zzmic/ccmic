#ifndef FRONTEND_DECLARATION_H
#define FRONTEND_DECLARATION_H

#include "ast.h"
#include "expression.h"
#include "storageClass.h"
#include "type.h"
#include <memory>
#include <optional>

namespace AST {
class Declaration : public AST {
  public:
    virtual ~Declaration() = default;
};

class VariableDeclaration : public Declaration {
  public:
    VariableDeclaration(const std::string &identifier,
                        std::shared_ptr<Type> varType);
    VariableDeclaration(
        const std::string &identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType);
    VariableDeclaration(
        const std::string &identifier, std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    VariableDeclaration(
        const std::string &identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::optional<std::shared_ptr<Expression>> getOptInitializer() const;
    std::shared_ptr<Type> getVarType() const;
    std::optional<std::shared_ptr<StorageClass>> getOptStorageClass() const;

  private:
    std::string identifier;
    std::optional<std::shared_ptr<Expression>> optInitializer;
    std::shared_ptr<Type> varType;
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};

class FunctionDeclaration : public Declaration {
  public:
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters,
                        std::shared_ptr<Type> funType);
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters,
                        std::optional<std::shared_ptr<Block>> optBody,
                        std::shared_ptr<Type> funType);
    FunctionDeclaration(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    FunctionDeclaration(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::shared_ptr<std::vector<std::string>> getParameterIdentifiers() const;
    std::shared_ptr<Type> getFunType() const;
    std::optional<std::shared_ptr<Block>> getOptBody() const;
    std::optional<std::shared_ptr<StorageClass>> getOptStorageClass() const;
    void setParameters(std::shared_ptr<std::vector<std::string>> parameters);
    void setOptBody(std::optional<std::shared_ptr<Block>> optBody);
    void setFunType(std::shared_ptr<Type> funType);
    void setOptStorageClass(
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::string>> parameters;
    std::optional<std::shared_ptr<Block>> optBody;
    std::shared_ptr<Type> funType;
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};
} // Namespace AST

#endif // FRONTEND_DECLARATION_H
