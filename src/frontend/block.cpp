#include "block.h"

namespace AST {
Block::Block(
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> blockItems)
    : blockItems(blockItems) {}

void Block::accept(Visitor &visitor) { visitor.visit(*this); }

const std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> &
Block::getBlockItems() const {
    return blockItems;
}

void Block::addBlockItem(std::shared_ptr<BlockItem> blockItem) {
    blockItems->push_back(blockItem);
}

void Block::setBlockItems(
    std::shared_ptr<std::vector<std::shared_ptr<BlockItem>>> newBlockItems) {
    this->blockItems = newBlockItems;
}
} // namespace AST
