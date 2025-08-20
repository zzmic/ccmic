#include "forInit.h"

namespace AST {
InitDecl::InitDecl(std::unique_ptr<VariableDeclaration> decl)
    : decl(std::move(decl)) {}

void InitDecl::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<VariableDeclaration> &InitDecl::getVariableDeclaration() {
    return decl;
}

InitExpr::InitExpr(std::optional<std::unique_ptr<Expression>> expr)
    : expr(std::move(expr)) {}

void InitExpr::accept(Visitor &visitor) { visitor.visit(*this); }

std::optional<std::unique_ptr<Expression>> &InitExpr::getExpression() {
    return expr;
}
} // Namespace AST
