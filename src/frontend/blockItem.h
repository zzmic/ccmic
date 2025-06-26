#ifndef FRONDEND_BLOCK_ITEM_H
#define FRONDEND_BLOCK_ITEM_H

#include "ast.h"
#include "declaration.h"
#include "statement.h"

namespace AST {
class BlockItem : public AST {
  public:
    constexpr BlockItem() = default;
};

class SBlockItem : public BlockItem {
  public:
    explicit SBlockItem(std::shared_ptr<Statement> statement);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Statement> getStatement() const;
    void setStatement(std::shared_ptr<Statement> statement);

  private:
    std::shared_ptr<Statement> statement;
};

class DBlockItem : public BlockItem {
  public:
    explicit DBlockItem(std::shared_ptr<Declaration> declaration);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Declaration> getDeclaration() const;
    void setDeclaration(std::shared_ptr<Declaration> declaration);

  private:
    std::shared_ptr<Declaration> declaration;
};
} // namespace AST

#endif // FRONDEND_BLOCK_ITEM_H
