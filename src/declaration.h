#ifndef DECLARATION_H
#define DECLARATION_H

#include "ast.h"
#include "expression.h"
#include <memory>
#include <optional>

namespace AST {
class Declaration : public AST {};

class VariableDeclaration : public Declaration {
  public:
    VariableDeclaration(const std::string &identifier);
    VariableDeclaration(const std::string &identifier,
                        std::optional<std::shared_ptr<Expression>> initializer);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::optional<std::shared_ptr<Expression>> getOptInitializer() const;

  private:
    std::string identifier;
    std::optional<std::shared_ptr<Expression>> optInitializer;
};

class FunctionDeclaration : public Declaration {
  public:
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters);
    FunctionDeclaration(const std::string &identifier,
                        std::shared_ptr<std::vector<std::string>> parameters,
                        std::optional<std::shared_ptr<Block>> optBody);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::shared_ptr<std::vector<std::string>> getParameters() const;
    std::optional<std::shared_ptr<Block>> getOptBody() const;
    void setOptBody(std::optional<std::shared_ptr<Block>> optBody);

  private:
    std::string identifier;
    std::shared_ptr<std::vector<std::string>> parameters;
    std::optional<std::shared_ptr<Block>> optBody;
};
} // Namespace AST

#endif // DECLARATION_H
