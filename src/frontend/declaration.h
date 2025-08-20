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
                                 std::unique_ptr<Type> varType);
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::unique_ptr<Expression>> optInitializer,
        std::unique_ptr<Type> varType);
    explicit VariableDeclaration(
        std::string_view identifier, std::unique_ptr<Type> varType,
        std::optional<std::unique_ptr<StorageClass>> optStorageClass);
    explicit VariableDeclaration(
        std::string_view identifier,
        std::optional<std::unique_ptr<Expression>> optInitializer,
        std::unique_ptr<Type> varType,
        std::optional<std::unique_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getIdentifier();
    [[nodiscard]] std::optional<std::unique_ptr<Expression>> &
    getOptInitializer();
    [[nodiscard]] std::unique_ptr<Type> &getVarType();
    [[nodiscard]] std::optional<std::unique_ptr<StorageClass>> &
    getOptStorageClass();

  private:
    std::string identifier;
    std::optional<std::unique_ptr<Expression>> optInitializer;
    std::unique_ptr<Type> varType;
    std::optional<std::unique_ptr<StorageClass>> optStorageClass;
};

class FunctionDeclaration : public Declaration {
  public:
    explicit FunctionDeclaration(std::string_view identifier,
                                 std::vector<std::string> parameters,
                                 std::unique_ptr<Type> funType);
    explicit FunctionDeclaration(std::string_view identifier,
                                 std::vector<std::string> parameters,
                                 std::optional<Block *> optBody,
                                 std::unique_ptr<Type> funType);
    explicit FunctionDeclaration(
        std::string_view identifier, std::vector<std::string> parameters,
        std::unique_ptr<Type> funType,
        std::optional<std::unique_ptr<StorageClass>> optStorageClass);
    explicit FunctionDeclaration(
        std::string_view identifier, std::vector<std::string> parameters,
        std::optional<Block *> optBody, std::unique_ptr<Type> funType,
        std::optional<std::unique_ptr<StorageClass>> optStorageClass);
    ~FunctionDeclaration();
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getIdentifier();
    [[nodiscard]] std::vector<std::string> &getParameterIdentifiers();
    [[nodiscard]] std::unique_ptr<Type> &getFunType();
    [[nodiscard]] std::optional<Block *> &getOptBody();
    [[nodiscard]] std::optional<std::unique_ptr<StorageClass>> &
    getOptStorageClass();
    void setParameters(std::vector<std::string> newParameters);
    void setOptBody(std::optional<Block *> newOptBody);
    void setFunType(std::unique_ptr<Type> newFunType);
    void setOptStorageClass(
        std::optional<std::unique_ptr<StorageClass>> newOptStorageClass);

  private:
    std::string identifier;
    std::vector<std::string> parameters;
    std::optional<Block *> optBody;
    std::unique_ptr<Type> funType;
    std::optional<std::unique_ptr<StorageClass>> optStorageClass;
};
} // Namespace AST

#endif // FRONTEND_DECLARATION_H
