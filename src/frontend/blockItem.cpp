#include "blockItem.h"
#include "visitor.h"

namespace AST {
SBlockItem::SBlockItem(std::unique_ptr<Statement> statement)
    : statement(std::move(statement)) {}

void SBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Statement> &SBlockItem::getStatement() { return statement; }

void SBlockItem::setStatement(std::unique_ptr<Statement> newStatement) {
    this->statement = std::move(newStatement);
}

DBlockItem::DBlockItem(std::unique_ptr<Declaration> declaration)
    : declaration(std::move(declaration)) {}

void DBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Declaration> &DBlockItem::getDeclaration() {
    return declaration;
}

void DBlockItem::setDeclaration(std::unique_ptr<Declaration> newDeclaration) {
    this->declaration = std::move(newDeclaration);
}
} // Namespace AST
