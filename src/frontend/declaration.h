#ifndef FRONTEND_DECLARATION_H
#define FRONTEND_DECLARATION_H

#include "ast.h"
#include "expression.h"
#include "storageClass.h"
#include "type.h"
#include <memory>
#include <optional>
#include <string_view>

namespace AST {
class Declaration : public AST {};

class VariableDeclaration : public Declaration {
  public:
    explicit VariableDeclaration(std::string_view identifier,
                                 std::shared_ptr<Type> varType);
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType);
    explicit VariableDeclaration(
        std::string_view identifier, std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::shared_ptr<Type> varType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getIdentifier() const;
    [[nodiscard]] std::optional<std::shared_ptr<Expression>>
    getOptInitializer() const;
    [[nodiscard]] std::shared_ptr<Type> getVarType() const;
    [[nodiscard]] std::optional<std::shared_ptr<StorageClass>>
    getOptStorageClass() const;

  private:
    std::string identifier;
    std::optional<std::shared_ptr<Expression>> optInitializer;
    std::shared_ptr<Type> varType;
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};

class FunctionDeclaration : public Declaration {
  public:
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<Type> funType);
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::shared_ptr<Type> funType);
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    explicit FunctionDeclaration(
        std::string_view identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::shared_ptr<Type> funType,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getIdentifier() const;
    [[nodiscard]] const std::shared_ptr<std::vector<std::string>> &
    getParameterIdentifiers() const;
    [[nodiscard]] std::shared_ptr<Type> getFunType() const;
    [[nodiscard]] std::optional<std::shared_ptr<Block>> getOptBody() const;
    [[nodiscard]] std::optional<std::shared_ptr<StorageClass>>
    getOptStorageClass() const;
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
