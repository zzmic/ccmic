#include "forInit.h"
#include <stdexcept>

namespace AST {
InitDecl::InitDecl(std::unique_ptr<VariableDeclaration> decl)
    : decl(std::move(decl)) {
    if (!this->decl) {
        throw std::logic_error("Creating InitDecl with null decl");
    }
}

void InitDecl::accept(Visitor &visitor) { visitor.visit(*this); }

VariableDeclaration *InitDecl::getVariableDeclaration() const {
    return decl.get();
}

InitExpr::InitExpr(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}

void InitExpr::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *InitExpr::getExpression() const { return expr.get(); }
} // Namespace AST
