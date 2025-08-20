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
    explicit ReturnStatement(std::unique_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getExpression();
    void setExpression(std::unique_ptr<Expression> expr);

  private:
    std::unique_ptr<Expression> expr;
};

class ExpressionStatement : public Statement {
  public:
    explicit ExpressionStatement(std::unique_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getExpression();
    void setExpression(std::unique_ptr<Expression> expr);

  private:
    std::unique_ptr<Expression> expr;
};

class IfStatement : public Statement {
  public:
    explicit IfStatement(
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> thenStatement,
        std::optional<std::unique_ptr<Statement>> elseOptStatement);
    explicit IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getCondition();
    [[nodiscard]] std::unique_ptr<Statement> &getThenStatement();
    [[nodiscard]] std::optional<std::unique_ptr<Statement>> &
    getElseOptStatement();
    void setCondition(std::unique_ptr<Expression> condition);
    void setThenStatement(std::unique_ptr<Statement> thenStatement);
    void setElseOptStatement(
        std::optional<std::unique_ptr<Statement>> elseOptStatement);

  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenStatement;
    std::optional<std::unique_ptr<Statement>> elseOptStatement;
};

class Block; // Forward declaration.

class CompoundStatement : public Statement {
  public:
    explicit CompoundStatement(Block *block);
    ~CompoundStatement();
    void accept(Visitor &visitor) override;
    [[nodiscard]] Block *getBlock();
    void setBlock(Block *block);

  private:
    Block *block;
};

class BreakStatement : public Statement {
  public:
    BreakStatement() : label("") {}
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getLabel();
    void setLabel(std::string_view label);

  private:
    std::string label;
};

class ContinueStatement : public Statement {
  public:
    ContinueStatement() : label("") {}
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::string &getLabel();
    void setLabel(std::string_view label);

  private:
    std::string label;
};

class WhileStatement : public Statement {
  public:
    explicit WhileStatement(std::unique_ptr<Expression> condition,
                            std::unique_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getCondition();
    [[nodiscard]] std::unique_ptr<Statement> &getBody();
    [[nodiscard]] std::string &getLabel();
    void setCondition(std::unique_ptr<Expression> condition);
    void setBody(std::unique_ptr<Statement> body);
    void setLabel(std::string_view label);

  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    std::string label;
};

class DoWhileStatement : public Statement {
  public:
    explicit DoWhileStatement(std::unique_ptr<Expression> condition,
                              std::unique_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<Expression> &getCondition();
    [[nodiscard]] std::unique_ptr<Statement> &getBody();
    [[nodiscard]] std::string &getLabel();
    void setCondition(std::unique_ptr<Expression> condition);
    void setBody(std::unique_ptr<Statement> body);
    void setLabel(std::string_view label);

  private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    std::string label;
};

class ForStatement : public Statement {
  public:
    explicit ForStatement(std::unique_ptr<ForInit> forInit,
                          std::optional<std::unique_ptr<Expression>> condition,
                          std::optional<std::unique_ptr<Expression>> post,
                          std::unique_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    [[nodiscard]] std::unique_ptr<ForInit> &getForInit();
    [[nodiscard]] std::optional<std::unique_ptr<Expression>> &getOptCondition();
    [[nodiscard]] std::optional<std::unique_ptr<Expression>> &getOptPost();
    [[nodiscard]] std::unique_ptr<Statement> &getBody();
    [[nodiscard]] std::string &getLabel();
    void setForInit(std::unique_ptr<ForInit> forInit);
    void
    setOptCondition(std::optional<std::unique_ptr<Expression>> optCondition);
    void setOptPost(std::optional<std::unique_ptr<Expression>> optPost);
    void setBody(std::unique_ptr<Statement> body);
    void setLabel(std::string_view label);

  private:
    std::unique_ptr<ForInit> forInit;
    std::optional<std::unique_ptr<Expression>> optCondition;
    std::optional<std::unique_ptr<Expression>> optPost;
    std::unique_ptr<Statement> body;
    std::string label;
};

class NullStatement : public Statement {
  public:
    constexpr NullStatement() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STATEMENT_H
