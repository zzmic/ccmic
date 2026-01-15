#ifndef FRONTEND_EXPRESSION_H
#define FRONTEND_EXPRESSION_H

#include "ast.h"
#include "constant.h"
#include "operator.h"
#include "type.h"
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
     * Default virtual destructor for the expression class.
     */
    virtual ~Expression() = default;

    /**
     * Pure virtual getter for the expression type.
     *
     * @return The type of the expression.
     */
    virtual std::shared_ptr<Type> getExpType() const = 0;

    /**
     * Pure virtual setter for the expression type.
     *
     * @param expType The new type of the expression.
     */
    virtual void setExpType(std::shared_ptr<Type> expType) = 0;
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
    explicit ConstantExpression(std::shared_ptr<Constant> constant);

    /**
     * Constructor for the constant expression class with expression type
     * information.
     *
     * @param constant The constant value of the expression.
     * @param expType The type of the expression.
     */
    explicit ConstantExpression(std::shared_ptr<Constant> constant,
                                std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Constant> getConstant() const;

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

    [[nodiscard]] int getConstantInInt() const;

    [[nodiscard]] std::variant<int, long> getConstantInIntOrLongVariant() const;

  private:
    /**
     * The constant value of the expression.
     */
    std::shared_ptr<Constant> constant;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
                                std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The identifier of the variable.
     */
    std::string identifier;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
    explicit CastExpression(std::shared_ptr<Type> targetType,
                            std::shared_ptr<Expression> expr);

    /**
     * Constructor for the cast expression class with expression type
     * information.
     *
     * @param targetType The target type of the cast expression.
     * @param expr The expression being casted.
     * @param expType The type of the expression.
     */
    explicit CastExpression(std::shared_ptr<Type> targetType,
                            std::shared_ptr<Expression> expr,
                            std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Type> getTargetType() const;

    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The target type of the cast expression.
     */
    std::shared_ptr<Type> targetType;

    /**
     * The expression being casted.
     */
    std::shared_ptr<Expression> expr;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
                             std::shared_ptr<Expression> expr);

    /**
     * Constructor for the unary expression class with operator as a string and
     * with expression type information.
     *
     * @param opInStr The unary operator as a string.
     * @param expr The expression being operated on.
     * @param expType The type of the expression.
     */
    explicit UnaryExpression(std::string_view opInStr,
                             std::shared_ptr<Expression> expr,
                             std::shared_ptr<Type> expType);

    /**
     * Constructor for the unary expression class with operator as an object and
     * without expression type information.
     *
     * @param op The unary operator.
     * @param expr The expression being operated on.
     */
    explicit UnaryExpression(std::shared_ptr<UnaryOperator> op,
                             std::shared_ptr<Factor> expr);

    /**
     * Constructor for the unary expression class with operator as an object and
     * with expression type information.
     *
     * @param op The unary operator.
     * @param expr The expression being operated on.
     * @param expType The type of the expression.
     */
    explicit UnaryExpression(std::shared_ptr<UnaryOperator> op,
                             std::shared_ptr<Factor> expr,
                             std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<UnaryOperator> getOperator() const;

    [[nodiscard]] std::shared_ptr<Factor> getExpression() const;

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The unary operator of the expression.
     */
    std::shared_ptr<UnaryOperator> op;

    /**
     * The expression being operated on.
     */
    std::shared_ptr<Factor> expr;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::string_view opInStr,
                              std::shared_ptr<Expression> right);

    /**
     * Constructor for the binary expression class with operator as a string and
     * with expression type information
     *
     * @param left The left operand of the binary expression.
     * @param opInStr The binary operator as a string.
     * @param right The right operand of the binary expression.
     * @param expType The type of the expression.
     */
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::string_view opInStr,
                              std::shared_ptr<Expression> right,
                              std::shared_ptr<Type> expType);
    /**
     * Constructor for the binary expression class with operator as an object
     * and without expression type information.
     *
     * @param left The left operand of the binary expression.
     * @param op The binary operator.
     * @param right The right operand of the binary expression.
     */
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::shared_ptr<BinaryOperator> op,
                              std::shared_ptr<Expression> right);

    /**
     * Constructor for the binary expression class with operator as an object
     * and with expression type information.
     *
     * @param left The left operand of the binary expression.
     * @param op The binary operator.
     * @param right The right operand of the binary expression.
     * @param expType The type of the expression.
     */
    explicit BinaryExpression(std::shared_ptr<Expression> left,
                              std::shared_ptr<BinaryOperator> op,
                              std::shared_ptr<Expression> right,
                              std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getLeft() const;

    void setLeft(std::shared_ptr<Expression> left);

    [[nodiscard]] std::shared_ptr<BinaryOperator> getOperator() const;

    void setOperator(std::shared_ptr<BinaryOperator> op);

    [[nodiscard]] std::shared_ptr<Expression> getRight() const;

    void setRight(std::shared_ptr<Expression> right);

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The left operand of the binary expression.
     */
    std::shared_ptr<Expression> left;

    /**
     * The binary operator of the expression.
     */
    std::shared_ptr<BinaryOperator> op;

    /**
     * The right operand of the binary expression.
     */
    std::shared_ptr<Expression> right;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
    explicit AssignmentExpression(std::shared_ptr<Expression> left,
                                  std::shared_ptr<Expression> right);

    /**
     * Constructor for the assignment expression class with expression type
     * information.
     *
     * @param left The left operand of the assignment expression.
     * @param right The right operand of the assignment expression.
     * @param expType The type of the expression.
     */
    explicit AssignmentExpression(std::shared_ptr<Expression> left,
                                  std::shared_ptr<Expression> right,
                                  std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getLeft() const;

    [[nodiscard]] std::shared_ptr<Expression> getRight() const;

    void setLeft(std::shared_ptr<Expression> left);

    void setRight(std::shared_ptr<Expression> right);

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The left operand of the assignment expression.
     */
    std::shared_ptr<Expression> left;

    /**
     * The right operand of the assignment expression.
     */
    std::shared_ptr<Expression> right;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
    explicit ConditionalExpression(std::shared_ptr<Expression> condition,
                                   std::shared_ptr<Expression> thenExpression,
                                   std::shared_ptr<Expression> elseExpression);

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
    explicit ConditionalExpression(std::shared_ptr<Expression> condition,
                                   std::shared_ptr<Expression> thenExpression,
                                   std::shared_ptr<Expression> elseExpression,
                                   std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;

    void setCondition(std::shared_ptr<Expression> condition);

    [[nodiscard]] std::shared_ptr<Expression> getThenExpression() const;

    void setThenExpression(std::shared_ptr<Expression> thenExpression);

    [[nodiscard]] std::shared_ptr<Expression> getElseExpression() const;

    void setElseExpression(std::shared_ptr<Expression> elseExpression);

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The condition expression of the conditional expression.
     */
    std::shared_ptr<Expression> condition;

    /**
     * The 'then' expression of the conditional expression.
     */
    std::shared_ptr<Expression> thenExpression;

    /**
     * The 'else' expression of the conditional expression.
     */
    std::shared_ptr<Expression> elseExpression;

    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
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
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);

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
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments,
        std::shared_ptr<Type> expType);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<Expression>>> &
    getArguments() const;

    void setArguments(
        std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments);

    [[nodiscard]] std::shared_ptr<Type> getExpType() const override;

    void setExpType(std::shared_ptr<Type> expType) override;

  private:
    /**
     * The identifier of the function being called.
     */
    std::string identifier;
    /**
     * The arguments of the function call.
     */
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> arguments;
    /**
     * The type of the expression.
     */
    std::shared_ptr<Type> expType;
};
} // Namespace AST

#endif // FRONTEND_EXPRESSION_H
