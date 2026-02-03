#include "blockItem.h"
#include "visitor.h"
#include <memory>
#include <stdexcept>
#include <utility>

namespace AST {
SBlockItem::SBlockItem(std::unique_ptr<Statement> statement)
    : statement(std::move(statement)) {
    if (!this->statement) {
        throw std::invalid_argument(
            "Creating SBlockItem with null statement in SBlockItem");
    }
}

void SBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

Statement *SBlockItem::getStatement() const { return statement.get(); }

void SBlockItem::setStatement(std::unique_ptr<Statement> newStatement) {
    statement = std::move(newStatement);
}

DBlockItem::DBlockItem(std::unique_ptr<Declaration> declaration)
    : declaration(std::move(declaration)) {
    if (!this->declaration) {
        throw std::invalid_argument(
            "Creating DBlockItem with null declaration in DBlockItem");
    }
}

void DBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

Declaration *DBlockItem::getDeclaration() const { return declaration.get(); }

void DBlockItem::setDeclaration(std::unique_ptr<Declaration> newDeclaration) {
    declaration = std::move(newDeclaration);
}
} // Namespace AST
