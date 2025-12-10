#ifndef FRONTEND_FOR_INIT_H
#define FRONTEND_FOR_INIT_H

#include "ast.h"
#include "declaration.h"
#include "expression.h"

namespace AST {
/**
 * Base class for for-loop initialization constructs in the AST.
 *
 * A for-loop initialization can be either a variable declaration or an
 * expression.
 */
class ForInit : public AST {
  public:
    constexpr ForInit() = default;
};

/**
 * Class representing a for-loop initialization with a variable declaration.
 */
class InitDecl : public ForInit {
  public:
    /**
     * Constructor for the init-decl class.
     *
     * @param decl The variable declaration used for initialization.
     */
    explicit InitDecl(std::shared_ptr<VariableDeclaration> decl);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<VariableDeclaration>
    getVariableDeclaration() const;

  private:
    /**
     * The variable declaration used for initialization.
     */
    std::shared_ptr<VariableDeclaration> decl;
};

/**
 * Class representing a for-loop initialization with an expression.
 */
class InitExpr : public ForInit {
  public:
    /**
     * Default constructor for the init-expr class.
     */
    constexpr InitExpr() = default;

    /**
     * Constructor for the init-expr class with an expression.
     *
     * @param expr The optional expression used for initialization.
     */
    explicit InitExpr(std::optional<std::shared_ptr<Expression>> expr);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::optional<std::shared_ptr<Expression>>
    getExpression() const;

  private:
    /**
     * The optional expression used for initialization.
     */
    std::optional<std::shared_ptr<Expression>> expr;
};
} // Namespace AST

#endif // FRONTEND_FOR_INIT_H
