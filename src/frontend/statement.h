#ifndef FRONTEND_STATEMENT_H
#define FRONTEND_STATEMENT_H

#include "ast.h"
#include "expression.h"
#include "forInit.h"
#include <memory>
#include <optional>
#include <string_view>

namespace AST {
class Block; // Forward declaration.

/**
 * Base class for statements in the AST.
 */
class Statement : public AST {
  public:
    /**
     * Default constructor of the statement class.
     */
    constexpr Statement() = default;
};

/**
 * Class representing a return statement in the AST.
 */
class ReturnStatement : public Statement {
  public:
    /**
     * Constructor of the return-statement class.
     *
     * @param expr The expression to return.
     */
    explicit ReturnStatement(std::shared_ptr<Expression> expr);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;

    void setExpression(std::shared_ptr<Expression> expr);

  private:
    /**
     * The expression to return.
     */
    std::shared_ptr<Expression> expr;
};

/**
 * Class representing an expression statement in the AST.
 */
class ExpressionStatement : public Statement {
  public:
    /**
     * Constructor of the expression-statement class.
     *
     * @param expr The expression in the statement.
     */
    explicit ExpressionStatement(std::shared_ptr<Expression> expr);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;

  private:
    /**
     * The expression in the statement.
     */
    std::shared_ptr<Expression> expr;
};

/**
 * Class representing an if statement in the AST.
 */
class IfStatement : public Statement {
  public:
    /**
     * Constructor of the if-statement class with an (optional) else statement.
     *
     * @param condition The condition expression of the if statement.
     * @param thenStatement The statement to execute if the condition is true.
     * @param elseOptStatement An optional statement to execute if the condition
     * is false.
     */
    explicit IfStatement(
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Statement> thenStatement,
        std::optional<std::shared_ptr<Statement>> elseOptStatement);

    /**
     * Constructor of the if-statement class without an (optional) else
     * statement.
     *
     * @param condition The condition expression of the if statement.
     * @param thenStatement The statement to execute if the condition is true.
     */
    explicit IfStatement(std::shared_ptr<Expression> condition,
                         std::shared_ptr<Statement> thenStatement);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;

    [[nodiscard]] std::shared_ptr<Statement> getThenStatement() const;

    [[nodiscard]] std::optional<std::shared_ptr<Statement>>
    getElseOptStatement() const;

  private:
    /**
     * The condition expression of the if statement.
     */
    std::shared_ptr<Expression> condition;

    /**
     * The statement to execute if the condition is true.
     */
    std::shared_ptr<Statement> thenStatement;

    /**
     * An optional statement to execute if the condition is false.
     */
    std::optional<std::shared_ptr<Statement>> elseOptStatement;
};

/**
 * Class representing a compound statement (block) in the AST.
 */
class CompoundStatement : public Statement {
  public:
    /**
     * Constructor of the compound-statement class.
     *
     * @param block The block of statements.
     */
    explicit CompoundStatement(std::shared_ptr<Block> block);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Block> getBlock() const;

  private:
    /**
     * The block of statements.
     */
    std::shared_ptr<Block> block;
};

/**
 * Class representing a break statement in the AST.
 */
class BreakStatement : public Statement {
  public:
    /**
     * Constructor of the break-statement class.
     */
    BreakStatement() : label("") {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The label of the loop to break from.
     */
    std::string label;
};

/**
 * Class representing a continue statement in the AST.
 */
class ContinueStatement : public Statement {
  public:
    /**
     * Constructor of the continue-statement class.
     */
    ContinueStatement() : label("") {}

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The label of the loop to continue.
     */
    std::string label;
};

/**
 * Class representing a while statement in the AST.
 */
class WhileStatement : public Statement {
  public:
    /**
     * Constructor of the while-statement class.
     *
     * @param condition The condition expression of the while statement.
     * @param body The body statement of the while loop.
     */
    explicit WhileStatement(std::shared_ptr<Expression> condition,
                            std::shared_ptr<Statement> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;

    [[nodiscard]] std::shared_ptr<Statement> getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The condition expression of the while statement.
     */
    std::shared_ptr<Expression> condition;

    /**
     * The body statement of the while loop.
     */
    std::shared_ptr<Statement> body;

    /**
     * The label of the loop.
     */
    std::string label;
};

/**
 * Class representing a do-while statement in the AST.
 */
class DoWhileStatement : public Statement {
  public:
    /**
     * Constructor of the do-while-statement class.
     *
     * @param condition The condition expression of the do-while statement.
     * @param body The body statement of the do-while loop.
     */
    explicit DoWhileStatement(std::shared_ptr<Expression> condition,
                              std::shared_ptr<Statement> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;

    [[nodiscard]] std::shared_ptr<Statement> getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The condition expression of the do-while statement.
     */
    std::shared_ptr<Expression> condition;

    /**
     * The body statement of the do-while loop.
     */
    std::shared_ptr<Statement> body;

    /**
     * The label of the loop.
     */
    std::string label;
};

/**
 * Class representing a for statement in the AST.
 */
class ForStatement : public Statement {
  public:
    /**
     * Constructor of the for-statement class.
     *
     * @param forInit The initialization part of the for statement.
     * @param condition The (optional) condition expression of the for
     * statement.
     * @param post The (optional) post-expression of the for statement.
     * @param body The body statement of the for loop.
     */
    explicit ForStatement(std::shared_ptr<ForInit> forInit,
                          std::optional<std::shared_ptr<Expression>> condition,
                          std::optional<std::shared_ptr<Expression>> post,
                          std::shared_ptr<Statement> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] std::shared_ptr<ForInit> getForInit() const;

    [[nodiscard]] std::optional<std::shared_ptr<Expression>>
    getOptCondition() const;

    [[nodiscard]] std::optional<std::shared_ptr<Expression>> getOptPost() const;

    [[nodiscard]] std::shared_ptr<Statement> getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The initialization part of the for statement.
     */
    std::shared_ptr<ForInit> forInit;

    /**
     * The (optional) condition expression of the for statement.
     */
    std::optional<std::shared_ptr<Expression>> optCondition;

    /**
     * The (optional) post-expression of the for statement.
     */
    std::optional<std::shared_ptr<Expression>> optPost;

    /**
     * The body statement of the for loop.
     */
    std::shared_ptr<Statement> body;

    /**
     * The label of the loop.
     */
    std::string label;
};

/**
 * Class representing a null statement in the AST.
 */
class NullStatement : public Statement {
  public:
    /**
     * Default constructor of the null-statement class.
     */
    constexpr NullStatement() = default;

    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STATEMENT_H
