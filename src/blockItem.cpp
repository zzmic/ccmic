#include "blockItem.h"
#include "visitor.h"

namespace AST {
SBlockItem::SBlockItem(std::shared_ptr<Statement> statement)
    : statement(statement) {}

void SBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Statement> SBlockItem::getStatement() const {
    return statement;
}

void SBlockItem::setStatement(std::shared_ptr<Statement> statement) {
    this->statement = statement;
}

DBlockItem::DBlockItem(std::shared_ptr<Declaration> declaration)
    : declaration(declaration) {}

void DBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Declaration> DBlockItem::getDeclaration() const {
    return declaration;
}

void DBlockItem::setDeclaration(std::shared_ptr<Declaration> declaration) {
    this->declaration = declaration;
}
} // Namespace AST
