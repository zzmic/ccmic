#ifndef DECLARATION_H
#define DECLARATION_H

#include "ast.h"
#include "expression.h"
#include <memory>
#include <optional>

namespace AST {
class Declaration : public AST {
  public:
    Declaration(const std::string &identifier);
    Declaration(const std::string &identifier,
                std::optional<std::shared_ptr<Expression>> initializer);
    void accept(Visitor &visitor) override;
    const std::string &getIdentifier() const;
    std::optional<std::shared_ptr<Expression>> getOptInitializer() const;

  private:
    std::string identifier;
    std::optional<std::shared_ptr<Expression>> optInitializer;
};
} // Namespace AST

#endif // DECLARATION_H
