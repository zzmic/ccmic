#ifndef FRONTEND_BLOCK_H
#define FRONTEND_BLOCK_H

#include "blockItem.h"
#include <memory>
#include <vector>

namespace AST {
class Block : public AST {
  public:
    explicit Block(std::vector<std::unique_ptr<BlockItem>> blockItems);
    ~Block() = default; // Vector destructor handles cleanup automatically.
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::vector<std::unique_ptr<BlockItem>> &getBlockItems();
    void addBlockItem(std::unique_ptr<BlockItem> blockItem);
    void setBlockItems(std::vector<std::unique_ptr<BlockItem>> blockItems);

  private:
    std::vector<std::unique_ptr<BlockItem>> blockItems;
};
} // namespace AST

#endif // FRONTEND_BLOCK_H
