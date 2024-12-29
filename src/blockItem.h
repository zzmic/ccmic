#ifndef BLOCK_ITEM_H
#define BLOCK_ITEM_H

#include "ast.h"
#include "declaration.h"
#include "statement.h"
#include <memory>

namespace AST {
class BlockItem : public AST {};

class SBlockItem : public BlockItem {
  public:
    SBlockItem(std::shared_ptr<Statement> statement);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Statement> getStatement() const;

  private:
    std::shared_ptr<Statement> statement;
};

class DBlockItem : public BlockItem {
  public:
    DBlockItem(std::shared_ptr<Declaration> declaration);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Declaration> getDeclaration() const;

  private:
    std::shared_ptr<Declaration> declaration;
};
} // namespace AST

#endif // BLOCK_ITEM_H
