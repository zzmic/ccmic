#ifndef FRONDEND_BLOCK_ITEM_H
#define FRONDEND_BLOCK_ITEM_H

#include "ast.h"
#include "declaration.h"
#include "statement.h"

namespace AST {
/**
 * Base class for block items in the AST.
 *
 * A block item can be either a statement or a declaration.
 */
class BlockItem : public AST {
  public:
    /**
     * Default constructor for the block item class.
     */
    constexpr BlockItem() = default;
};

/**
 * Class representing a statement block item in the AST.
 */
class SBlockItem : public BlockItem {
  public:
    /**
     * Constructor for the statement block item class.
     *
     * @param statement The statement encapsulated by the block item.
     */
    explicit SBlockItem(std::unique_ptr<Statement> statement);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Statement *getStatement() const;

    void setStatement(std::unique_ptr<Statement> statement);

  private:
    /**
     * The statement encapsulated by the block item.
     */
    std::unique_ptr<Statement> statement;
};

/**
 * Class representing a declaration block item in the AST.
 */
class DBlockItem : public BlockItem {
  public:
    /**
     * Constructor for the declaration block item class.
     *
     * @param declaration The declaration encapsulated by the block item.
     */
    explicit DBlockItem(std::unique_ptr<Declaration> declaration);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Declaration *getDeclaration() const;

    void setDeclaration(std::unique_ptr<Declaration> declaration);

  private:
    /**
     * The declaration encapsulated by the block item.
     */
    std::unique_ptr<Declaration> declaration;
};
} // namespace AST

#endif // FRONDEND_BLOCK_ITEM_H
