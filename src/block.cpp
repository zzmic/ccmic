#include "block.h"

namespace AST {
Block::Block(
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems)
    : blockItems(blockItems) {}

void Block::accept(Visitor &visitor) { visitor.visit(*this); }

void Block::addBlockItem(std::shared_ptr<BlockItem> blockItem) {
    blockItems->emplace_back(blockItem);
}

std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>>
Block::getBlockItems() const {
    return blockItems;
}

void Block::setBlockItems(
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems) {
    this->blockItems = blockItems;
}
} // namespace AST
