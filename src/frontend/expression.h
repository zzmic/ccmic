#ifndef FRONTEND_EXPRESSION_H
#define FRONTEND_EXPRESSION_H

#include "ast.h"
#include "constant.h"
#include "operator.h"
#include "type.h"
#include "visitor.h"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace AST {
/**
 * Base class for expressions in the AST.
 *
 * An expression can be a factor expression (constant expression, variable
 * expression, cast expression, or unary expression), binary expression,
 * assignment expression, conditional expression, or function call expression.
 */
class Expression : public AST {
  public:
    /**
     * Default constructor for the expression class.
     */
    constexpr Expression() = default;

    /**
     * Default destructor for the expression class.
     */
    ~Expression() override = default;

    /**
     * Delete the copy constructor for the expression class.
     */
    constexpr Expression(const Expression &) = delete;

    /**
     * Delete the copy assignment operator for the expression class.
     */
    constexpr Expression &operator=(const Expression &) = delete;

    /**
     * Default move constructor for the expression class.
     */
    constexpr Expression(Expression &&) = default;

    /**
     * Default move assignment operator for the expression class.
     */
    constexpr Expression &operator=(Expression &&) = default;

    /**
     * Pure virtual getter for the expression type.
     *
     * @return The type of the expression (non-owning pointer).
     */
    virtual Type *getExpType() const = 0;

    /**
     * Pure virtual setter for the expression type.
     *
     * @param expType The new type of the expression.
     */
    virtual void setExpType(std::unique_ptr<Type> expType) = 0;
};

/**
 * Base class for factors in the AST.
 *
 * A factor can be a constant expression, variable expression, cast expression,
 * or unary expression.
 */
class Factor : public Expression {};

/**
 * Class representing a constant expression.
 */
class ConstantExpression : public Factor {
  public:
    /**
     * Constructor for the constant expression class without expression type
     * information.
     *
     * @param constant The constant value of the expression.
     */
    explicit ConstantExpression(std::unique_ptr<Constant> constant);

    /**
     * Constructor for the constant expression class with expression type
     * information.
     *
     * @param constant The constant value of the expression.
     * @param expType The type of the expression.
     */
    explicit ConstantExpression(std::unique_ptr<Constant> constant,
                                std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Constant *getConstant() const;

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

    [[nodiscard]] std::variant<int, long, unsigned int, unsigned long>
    getConstantInVariant() const;

  private:
    /**
     * The constant value of the expression.
     */
    std::unique_ptr<Constant> constant;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

class VariableExpression : public Factor {
  public:
    /**
     * Constructor for the variable expression class without expression type
     * information.
     *
     * @param identifier The identifier of the variable.
     */
    explicit VariableExpression(std::string_view identifier);

    /**
     * Constructor for the variable expression class with expression type
     * information.
     *
     * @param identifier The identifier of the variable.
     * @param expType The type of the expression.
     */
    explicit VariableExpression(std::string_view identifier,
                                std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    void setIdentifier(std::string_view identifier);

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The identifier of the variable.
     */
    std::string identifier;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing a cast expression.
 */
class CastExpression : public Factor {
  public:
    /**
     * Constructor for the cast expression class without expression type
     * information.
     *
     * @param targetType The target type of the cast expression.
     */
    explicit CastExpression(std::unique_ptr<Type> targetType,
                            std::unique_ptr<Expression> expr);

    /**
     * Constructor for the cast expression class with expression type
     * information.
     *
     * @param targetType The target type of the cast expression.
     * @param expr The expression being casted.
     * @param expType The type of the expression.
     */
    explicit CastExpression(std::unique_ptr<Type> targetType,
                            std::unique_ptr<Expression> expr,
                            std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Type *getTargetType() const;

    [[nodiscard]] Expression *getExpression() const;

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The target type of the cast expression.
     */
    std::unique_ptr<Type> targetType;

    /**
     * The expression being casted.
     */
    std::unique_ptr<Expression> expr;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing a unary expression.
 */
class UnaryExpression : public Factor {
  public:
    /**
     * Constructor for the unary expression class with operator as a string and
     * without expression type information.
     *
     * @param opInStr The unary operator as a string.
     * @param expr The expression being operated on.
     */
    explicit UnaryExpression(std::string_view opInStr,
                             std::unique_ptr<Expression> expr);

    /**
     * Constructor for the unary expression class with operator as a string and
     * with expression type information.
     *
     * @param opInStr The unary operator as a string.
     * @param expr The expression being operated on.
     * @param expType The type of the expression.
     */
    explicit UnaryExpression(std::string_view opInStr,
                             std::unique_ptr<Expression> expr,
                             std::unique_ptr<Type> expType);

    /**
     * Constructor for the unary expression class with operator as an object and
     * without expression type information.
     *
     * @param op The unary operator.
     * @param expr The expression being operated on.
     */
    explicit UnaryExpression(std::unique_ptr<UnaryOperator> op,
                             std::unique_ptr<Expression> expr);

    /**
     * Constructor for the unary expression class with operator as an object and
     * with expression type information.
     *
     * @param op The unary operator.
     * @param expr The expression being operated on.
     * @param expType The type of the expression.
     */
    explicit UnaryExpression(std::unique_ptr<UnaryOperator> op,
                             std::unique_ptr<Expression> expr,
                             std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] UnaryOperator *getOperator() const;

    [[nodiscard]] Expression *getExpression() const;

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The unary operator of the expression.
     */
    std::unique_ptr<UnaryOperator> op;

    /**
     * The expression being operated on.
     */
    std::unique_ptr<Expression> expr;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing a binary expression.
 */
class BinaryExpression : public Expression {
  public:
    /**
     * Constructor for the binary expression class with operator as a string and
     * without expression type information.
     *
     * @param left The left operand of the binary expression.
     * @param opInStr The binary operator as a string.
     * @param right The right operand of the binary expression.
     */
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::string_view opInStr,
                              std::unique_ptr<Expression> right);

    /**
     * Constructor for the binary expression class with operator as a string and
     * with expression type information
     *
     * @param left The left operand of the binary expression.
     * @param opInStr The binary operator as a string.
     * @param right The right operand of the binary expression.
     * @param expType The type of the expression.
     */
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::string_view opInStr,
                              std::unique_ptr<Expression> right,
                              std::unique_ptr<Type> expType);
    /**
     * Constructor for the binary expression class with operator as an object
     * and without expression type information.
     *
     * @param left The left operand of the binary expression.
     * @param op The binary operator.
     * @param right The right operand of the binary expression.
     */
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::unique_ptr<BinaryOperator> op,
                              std::unique_ptr<Expression> right);

    /**
     * Constructor for the binary expression class with operator as an object
     * and with expression type information.
     *
     * @param left The left operand of the binary expression.
     * @param op The binary operator.
     * @param right The right operand of the binary expression.
     * @param expType The type of the expression.
     */
    explicit BinaryExpression(std::unique_ptr<Expression> left,
                              std::unique_ptr<BinaryOperator> op,
                              std::unique_ptr<Expression> right,
                              std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getLeft() const;

    void setLeft(std::unique_ptr<Expression> left);

    [[nodiscard]] BinaryOperator *getOperator() const;

    void setOperator(std::unique_ptr<BinaryOperator> op);

    [[nodiscard]] Expression *getRight() const;

    void setRight(std::unique_ptr<Expression> right);

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The left operand of the binary expression.
     */
    std::unique_ptr<Expression> left;

    /**
     * The binary operator of the expression.
     */
    std::unique_ptr<BinaryOperator> op;

    /**
     * The right operand of the binary expression.
     */
    std::unique_ptr<Expression> right;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing an assignment expression.
 */
class AssignmentExpression : public Expression {
  public:
    /**
     * Constructor for the assignment expression class without expression type
     * information.
     *
     * @param left The left operand of the assignment expression.
     * @param right The right operand of the assignment expression.
     */
    explicit AssignmentExpression(std::unique_ptr<Expression> left,
                                  std::unique_ptr<Expression> right);

    /**
     * Constructor for the assignment expression class with expression type
     * information.
     *
     * @param left The left operand of the assignment expression.
     * @param right The right operand of the assignment expression.
     * @param expType The type of the expression.
     */
    explicit AssignmentExpression(std::unique_ptr<Expression> left,
                                  std::unique_ptr<Expression> right,
                                  std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getLeft() const;

    [[nodiscard]] Expression *getRight() const;

    void setLeft(std::unique_ptr<Expression> left);

    void setRight(std::unique_ptr<Expression> right);

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The left operand of the assignment expression.
     */
    std::unique_ptr<Expression> left;

    /**
     * The right operand of the assignment expression.
     */
    std::unique_ptr<Expression> right;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing a conditional expression.
 */
class ConditionalExpression : public Expression {
  public:
    /**
     * Constructor for the conditional expression class without expression type
     * information.
     *
     * @param condition The condition expression of the conditional expression.
     * @param thenExpression The 'then' expression of the conditional
     * expression.
     * @param elseExpression The 'else' expression of the conditional
     * expression.
     */
    explicit ConditionalExpression(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Expression> thenExpression,
                                   std::unique_ptr<Expression> elseExpression);

    /**
     * Constructor for the conditional expression class with expression type
     * information.
     *
     * @param condition The condition expression of the conditional expression.
     * @param thenExpression The 'then' expression of the conditional
     * expression.
     * @param elseExpression The 'else' expression of the conditional
     * expression.
     * @param expType The type of the expression.
     */
    explicit ConditionalExpression(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Expression> thenExpression,
                                   std::unique_ptr<Expression> elseExpression,
                                   std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getCondition() const;

    void setCondition(std::unique_ptr<Expression> condition);

    [[nodiscard]] Expression *getThenExpression() const;

    void setThenExpression(std::unique_ptr<Expression> thenExpression);

    [[nodiscard]] Expression *getElseExpression() const;

    void setElseExpression(std::unique_ptr<Expression> elseExpression);

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The condition expression of the conditional expression.
     */
    std::unique_ptr<Expression> condition;

    /**
     * The 'then' expression of the conditional expression.
     */
    std::unique_ptr<Expression> thenExpression;

    /**
     * The 'else' expression of the conditional expression.
     */
    std::unique_ptr<Expression> elseExpression;

    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};

/**
 * Class representing a function call expression.
 */
class FunctionCallExpression : public Expression {
  public:
    /**
     * Constructor for the function call expression class without expression
     * type information.
     *
     * @param identifier The identifier of the function being called.
     * @param arguments The arguments of the function call.
     */
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments);

    /**
     * Constructor for the function call expression class with expression type
     * information.
     *
     * @param identifier The identifier of the function being called.
     * @param arguments The arguments of the function call.
     * @param expType The type of the expression.
     */
    explicit FunctionCallExpression(
        std::string_view identifier,
        std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments,
        std::unique_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    void setIdentifier(std::string_view identifier);

    [[nodiscard]] const std::vector<std::unique_ptr<Expression>> &
    getArguments() const;

    void setArguments(
        std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments);

    [[nodiscard]] Type *getExpType() const override;

    void setExpType(std::unique_ptr<Type> expType) override;

  private:
    /**
     * The identifier of the function being called.
     */
    std::string identifier;
    /**
     * The arguments of the function call.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Expression>>> arguments;
    /**
     * The type of the expression.
     */
    std::unique_ptr<Type> expType;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
