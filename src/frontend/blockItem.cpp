#include "blockItem.h"
#include "visitor.h"

namespace AST {
SBlockItem::SBlockItem(std::unique_ptr<Statement> statement)
    : statement(std::move(statement)) {}

void SBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

Statement *SBlockItem::getStatement() const { return statement.get(); }

void SBlockItem::setStatement(std::unique_ptr<Statement> newStatement) {
    this->statement = std::move(newStatement);
}

DBlockItem::DBlockItem(std::unique_ptr<Declaration> declaration)
    : declaration(std::move(declaration)) {}

void DBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

Declaration *DBlockItem::getDeclaration() const { return declaration.get(); }

void DBlockItem::setDeclaration(std::unique_ptr<Declaration> newDeclaration) {
    this->declaration = std::move(newDeclaration);
}
} // Namespace AST
