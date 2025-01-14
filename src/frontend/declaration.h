#ifndef FRONTEND_DECLARATION_H
#define FRONTEND_DECLARATION_H

#include "ast.h"
#include "expression.h"
#include "storageClass.h"
#include <memory>
#include <optional>

namespace AST {
class Declaration : public AST {
  public:
    virtual ~Declaration() = default;
};

class VariableDeclaration : public Declaration {
  public:
    VariableDeclaration(const std::string &identifier);
    VariableDeclaration(
        const std::string &identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer);
    VariableDeclaration(
        const std::string &identifier,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    VariableDeclaration(
        const std::string &identifier,
        std::optional<std::shared_ptr<Expression>> optInitializer,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::optional<std::shared_ptr<Expression>> getOptInitializer() const;
    std::optional<std::shared_ptr<StorageClass>> getOptStorageClass() const;

  private:
    std::string identifier;
    std::optional<std::shared_ptr<Expression>> optInitializer;
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};

class FunctionDeclaration : public Declaration {
  public:
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters);
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters,
                        std::optional<std::shared_ptr<Block>> optBody);
    FunctionDeclaration(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    FunctionDeclaration(
        const std::string &identifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::optional<std::shared_ptr<Block>> optBody,
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::shared_ptr<std::vector<std::string>> getParameters() const;
    std::optional<std::shared_ptr<Block>> getOptBody() const;
    void setOptBody(std::optional<std::shared_ptr<Block>> optBody);
    std::optional<std::shared_ptr<StorageClass>> getOptStorageClass() const;
    void setOptStorageClass(
        std::optional<std::shared_ptr<StorageClass>> optStorageClass);

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::string>> parameters;
    std::optional<std::shared_ptr<Block>> optBody;
    std::optional<std::shared_ptr<StorageClass>> optStorageClass;
};
} // Namespace AST

#endif // FRONTEND_DECLARATION_H
