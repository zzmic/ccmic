#ifndef FRONTEND_STATEMENT_H
#define FRONTEND_STATEMENT_H

#include "ast.h"
#include "expression.h"
#include "forInit.h"
#include <memory>
#include <optional>
#include <string_view>

namespace AST {
// Forward declaration.
class Block;

class Statement : public AST {
  public:
    constexpr Statement() = default;
};

class ReturnStatement : public Statement {
  public:
    explicit ReturnStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;
    void setExpression(std::shared_ptr<Expression> expr);

  private:
    std::shared_ptr<Expression> expr;
};

class ExpressionStatement : public Statement {
  public:
    explicit ExpressionStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Expression> expr;
};

class IfStatement : public Statement {
  public:
    explicit IfStatement(
        std::shared_ptr<Expression> condition,
        std::shared_ptr<Statement> thenStatement,
        std::optional<std::shared_ptr<Statement>> elseOptStatement);
    explicit IfStatement(std::shared_ptr<Expression> condition,
                         std::shared_ptr<Statement> thenStatement);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;
    [[nodiscard]] std::shared_ptr<Statement> getThenStatement() const;
    [[nodiscard]] std::optional<std::shared_ptr<Statement>>
    getElseOptStatement() const;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenStatement;
    std::optional<std::shared_ptr<Statement>> elseOptStatement;
};

class CompoundStatement : public Statement {
  public:
    explicit CompoundStatement(std::shared_ptr<Block> block);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Block> getBlock() const;

  private:
    std::shared_ptr<Block> block;
};

class BreakStatement : public Statement {
  public:
    BreakStatement() : label("") {}
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getLabel() const;
    void setLabel(std::string_view label);

  private:
    std::string label;
};

class ContinueStatement : public Statement {
  public:
    ContinueStatement() : label("") {}
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::string &getLabel() const;
    void setLabel(std::string_view label);

  private:
    std::string label;
};

class WhileStatement : public Statement {
  public:
    explicit WhileStatement(std::shared_ptr<Expression> condition,
                            std::shared_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;
    [[nodiscard]] std::shared_ptr<Statement> getBody() const;
    [[nodiscard]] const std::string &getLabel() const;
    void setLabel(std::string_view label);

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
    std::string label;
};

class DoWhileStatement : public Statement {
  public:
    explicit DoWhileStatement(std::shared_ptr<Expression> condition,
                              std::shared_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::shared_ptr<Expression> getCondition() const;
    [[nodiscard]] std::shared_ptr<Statement> getBody() const;
    [[nodiscard]] const std::string &getLabel() const;
    void setLabel(std::string_view label);

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
    std::string label;
};

class ForStatement : public Statement {
  public:
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
    std::shared_ptr<ForInit> forInit;
    std::optional<std::shared_ptr<Expression>> optCondition;
    std::optional<std::shared_ptr<Expression>> optPost;
    std::shared_ptr<Statement> body;
    std::string label;
};

class NullStatement : public Statement {
  public:
    constexpr NullStatement() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STATEMENT_H
