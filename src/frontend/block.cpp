#include "block.h"
#include <stdexcept>

namespace AST {
Block::Block(
    std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> blockItems)
    : blockItems(std::move(blockItems)) {
    if (!this->blockItems) {
        throw std::logic_error("Creating Block with null blockItems");
    }
}

void Block::accept(Visitor &visitor) { visitor.visit(*this); }

const std::vector<std::unique_ptr<BlockItem>> &Block::getBlockItems() const {
    return *blockItems;
}

void Block::setBlockItems(
    std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> newBlockItems) {
    blockItems = std::move(newBlockItems);
}

void Block::addBlockItem(std::unique_ptr<BlockItem> blockItem) {
    blockItems->push_back(std::move(blockItem));
}
} // namespace AST
