#include "block.h"
#include "blockItem.h"
#include "visitor.h"
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace AST {
Block::Block(
    std::unique_ptr<std::vector<std::unique_ptr<BlockItem>>> blockItems)
    : blockItems(std::move(blockItems)) {
    if (!this->blockItems) {
        throw std::invalid_argument(
            "Creating Block with null blockItems in Block");
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
