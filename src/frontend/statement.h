#ifndef FRONTEND_STATEMENT_H
#define FRONTEND_STATEMENT_H

#include "ast.h"
#include "expression.h"
#include "visitor.h"
#include <memory>
#include <string>
#include <string_view>

namespace AST {
// Forward declaration of `Block` to avoid circular dependencies.
class Block;
// Forward declaration of `ForInit` to avoid circular dependencies.
class ForInit;

/**
 * Base class for statements in the AST.
 */
class Statement : public AST {};

/**
 * Class representing a return statement in the AST.
 */
class ReturnStatement : public Statement {
  public:
    /**
     * Constructor for the return-statement class.
     *
     * @param expr The expression to return.
     */
    explicit ReturnStatement(std::unique_ptr<Expression> expr);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getExpression() const;

    void setExpression(std::unique_ptr<Expression> expr);

  private:
    /**
     * The expression to return.
     */
    std::unique_ptr<Expression> expr;
};

/**
 * Class representing an expression statement in the AST.
 */
class ExpressionStatement : public Statement {
  public:
    /**
     * Constructor for the expression-statement class.
     *
     * @param expr The expression in the statement.
     */
    explicit ExpressionStatement(std::unique_ptr<Expression> expr);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getExpression() const;

  private:
    /**
     * The expression in the statement.
     */
    std::unique_ptr<Expression> expr;
};

/**
 * Class representing an if statement in the AST.
 */
class IfStatement : public Statement {
  public:
    /**
     * Constructor for the if-statement class with an (optional) else statement.
     *
     * @param condition The condition expression of the if statement.
     * @param thenStatement The statement to execute if the condition is true.
     * @param elseOptStatement An optional statement to execute if the condition
     * is false.
     */
    explicit IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement,
                         std::unique_ptr<Statement> elseOptStatement);

    /**
     * Constructor for the if-statement class without an (optional) else
     * statement.
     *
     * @param condition The condition expression of the if statement.
     * @param thenStatement The statement to execute if the condition is true.
     */
    explicit IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getCondition() const;

    [[nodiscard]] Statement *getThenStatement() const;

    [[nodiscard]] Statement *getElseOptStatement() const;

  private:
    /**
     * The condition expression of the if statement.
     */
    std::unique_ptr<Expression> condition;

    /**
     * The statement to execute if the condition is true.
     */
    std::unique_ptr<Statement> thenStatement;

    /**
     * An optional statement to execute if the condition is false (can be
     * `nullptr`).
     */
    std::unique_ptr<Statement> elseOptStatement;
};

/**
 * Class representing a compound statement (block) in the AST.
 */
class CompoundStatement : public Statement {
  public:
    /**
     * Constructor for the compound statement class.
     *
     * @param block The block of statements.
     */
    explicit CompoundStatement(std::unique_ptr<Block> block);

    /**
     * (Default) destructor for the `CompoundStatement` class.
     *
     * Defined in `statement.cpp` to allow incomplete type `Block` in the
     * header.
     */
    ~CompoundStatement() override;

    /**
     * Delete the copy constructor for the compound statement class.
     */
    constexpr CompoundStatement(const CompoundStatement &) = delete;

    /**
     * Delete the copy assignment operator for the compound statement class.
     */
    constexpr CompoundStatement &operator=(const CompoundStatement &) = delete;

    /**
     * Default move constructor for the compound statement class.
     */
    constexpr CompoundStatement(CompoundStatement &&) = default;

    /**
     * Default move assignment operator for the compound statement class.
     */
    constexpr CompoundStatement &operator=(CompoundStatement &&) = default;

    void accept(Visitor &visitor) override;

    [[nodiscard]] Block *getBlock() const;

  private:
    /**
     * The block of statements.
     */
    std::unique_ptr<Block> block;
};

/**
 * Class representing a break statement in the AST.
 */
class BreakStatement : public Statement {
  public:
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
     * Constructor for the while-statement class.
     *
     * @param condition The condition expression of the while statement.
     * @param body The body statement of the while loop.
     */
    explicit WhileStatement(std::unique_ptr<Expression> condition,
                            std::unique_ptr<Statement> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getCondition() const;

    [[nodiscard]] Statement *getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The condition expression of the while statement.
     */
    std::unique_ptr<Expression> condition;

    /**
     * The body statement of the while loop.
     */
    std::unique_ptr<Statement> body;

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
     * Constructor for the do-while-statement class.
     *
     * @param condition The condition expression of the do-while statement.
     * @param body The body statement of the do-while loop.
     */
    explicit DoWhileStatement(std::unique_ptr<Expression> condition,
                              std::unique_ptr<Statement> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] Expression *getCondition() const;

    [[nodiscard]] Statement *getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The condition expression of the do-while statement.
     */
    std::unique_ptr<Expression> condition;

    /**
     * The body statement of the do-while loop.
     */
    std::unique_ptr<Statement> body;

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
     * Constructor for the for-statement class.
     *
     * @param forInit The initialization part of the for statement.
     * @param condition The (optional) condition expression of the for
     * statement (can be `nullptr`).
     * @param post The (optional) post-expression of the for statement (can be
     * `nullptr`).
     * @param body The body statement of the for loop.
     */
    explicit ForStatement(std::unique_ptr<ForInit> forInit,
                          std::unique_ptr<Expression> condition,
                          std::unique_ptr<Expression> post,
                          std::unique_ptr<Statement> body);

    /**
     * (Default) destructor for the `ForStatement` class.
     *
     * Defined in `statement.cpp` to allow incomplete type `ForInit` in the
     * header.
     */
    ~ForStatement() override;

    /**
     * Delete the copy constructor for the for-statement class.
     */
    constexpr ForStatement(const ForStatement &) = delete;

    /**
     * Delete the copy assignment operator for the for-statement class.
     */
    constexpr ForStatement &operator=(const ForStatement &) = delete;

    /**
     * Default move constructor for the for-statement class.
     */
    constexpr ForStatement(ForStatement &&) = default;

    /**
     * Default move assignment operator for the for-statement class.
     */
    constexpr ForStatement &operator=(ForStatement &&) = default;

    void accept(Visitor &visitor) override;

    [[nodiscard]] ForInit *getForInit() const;

    [[nodiscard]] Expression *getOptCondition() const;

    [[nodiscard]] Expression *getOptPost() const;

    [[nodiscard]] Statement *getBody() const;

    [[nodiscard]] const std::string &getLabel() const;

    void setLabel(std::string_view label);

  private:
    /**
     * The initialization part of the for statement.
     */
    std::unique_ptr<ForInit> forInit;

    /**
     * The (optional) condition expression of the for statement (can be
     * `nullptr`).
     */
    std::unique_ptr<Expression> optCondition;

    /**
     * The (optional) post-expression of the for statement (can be `nullptr`).
     */
    std::unique_ptr<Expression> optPost;

    /**
     * The body statement of the for loop.
     */
    std::unique_ptr<Statement> body;

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
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STATEMENT_H
