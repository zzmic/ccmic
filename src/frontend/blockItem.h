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
    explicit SBlockItem(std::unique_ptr<Statement> statement);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Statement> &getStatement();
    void setStatement(std::unique_ptr<Statement> statement);

  private:
    std::unique_ptr<Statement> statement;
};

class DBlockItem : public BlockItem {
  public:
    explicit DBlockItem(std::unique_ptr<Declaration> declaration);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Declaration> &getDeclaration();
    void setDeclaration(std::unique_ptr<Declaration> declaration);

  private:
    std::unique_ptr<Declaration> declaration;
};
} // namespace AST

#endif // FRONDEND_BLOCK_ITEM_H
