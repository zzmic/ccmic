#include "forInit.h"

namespace AST {
InitDecl::InitDecl(std::shared_ptr<VariableDeclaration> decl) : decl(decl) {}

void InitDecl::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<VariableDeclaration> InitDecl::getVariableDeclaration() const {
    return decl;
}

InitExpr::InitExpr(std::optional<std::shared_ptr<Expression>> expr)
    : expr(expr) {}

void InitExpr::accept(Visitor &visitor) { visitor.visit(*this); }

std::optional<std::shared_ptr<Expression>> InitExpr::getExpression() const {
    return expr;
}
} // Namespace AST
