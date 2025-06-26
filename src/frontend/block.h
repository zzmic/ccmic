#ifndef FRONTEND_BLOCK_H
#define FRONTEND_BLOCK_H

#include "blockItem.h"
#include <vector>

namespace AST {
class Block : public AST {
  public:
    explicit Block(
        std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<BlockItem>>> &
    getBlockItems() const;
    void addBlockItem(std::shared_ptr<BlockItem> blockItem);
    void setBlockItems(
        std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems);

  private:
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems;
};
} // namespace AST

#endif // FRONTEND_BLOCK_H
