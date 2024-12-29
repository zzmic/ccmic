#include "blockItem.h"
#include "visitor.h"

namespace AST {
SBlockItem::SBlockItem(std::shared_ptr<Statement> statement)
    : statement(statement) {}

void SBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Statement> SBlockItem::getStatement() const {
    return statement;
}

DBlockItem::DBlockItem(std::shared_ptr<Declaration> declaration)
    : declaration(declaration) {}

void DBlockItem::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Declaration> DBlockItem::getDeclaration() const {
    return declaration;
}
} // Namespace AST
