#include "block.h"
#include <memory>

namespace AST {
Block::Block(std::vector<std::unique_ptr<BlockItem>> blockItems)
    : blockItems(std::move(blockItems)) {}

void Block::accept(Visitor &visitor) { visitor.visit(*this); }

std::vector<std::unique_ptr<BlockItem>> &Block::getBlockItems() {
    return blockItems;
}

void Block::addBlockItem(std::unique_ptr<BlockItem> blockItem) {
    blockItems.push_back(std::move(blockItem));
}

void Block::setBlockItems(
    std::vector<std::unique_ptr<BlockItem>> newBlockItems) {
    this->blockItems = std::move(newBlockItems);
}
} // namespace AST
