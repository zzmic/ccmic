#ifndef FRONTEND_BLOCK_H
#define FRONTEND_BLOCK_H

#include "blockItem.h"
#include <vector>

namespace AST {
/**
 * Class representing a block of code, containing a list of block items.
 */
class Block : public AST {
  public:
    /**
     * Constructor for the block class.
     *
     * @param blockItems The list of block items in the block.
     */
    explicit Block(
        std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> blockItems);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::vector<std::unique_ptr<BlockItem>> &
    getBlockItems() const;

    void setBlockItems(
        std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> blockItems);

    /**
     * Adds a block item to the block.
     *
     * @param blockItem The block item to add.
     */
    void addBlockItem(std::unique_ptr<BlockItem> blockItem);

  private:
    /**
     * The list of block items in the block.
     */
    std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> blockItems;
};
} // namespace AST

#endif // FRONTEND_BLOCK_H
