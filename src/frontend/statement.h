#ifndef FRONTEND_STATEMENT_H
#define FRONTEND_STATEMENT_H

#include "ast.h"
#include "block.h"
#include "expression.h"
#include "forInit.h"
#include <memory>
#include <optional>

namespace AST {
class Statement : public AST {};

class ReturnStatement : public Statement {
  public:
    ReturnStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Expression> expr;
};

class ExpressionStatement : public Statement {
  public:
    ExpressionStatement(std::shared_ptr<Expression> expr);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getExpression() const;

  private:
    std::shared_ptr<Expression> expr;
};

class IfStatement : public Statement {
  public:
    IfStatement(std::shared_ptr<Expression> condition,
                std::shared_ptr<Statement> thenStatement,
                std::optional<std::shared_ptr<Statement>> elseOptStatement);
    IfStatement(std::shared_ptr<Expression> condition,
                std::shared_ptr<Statement> thenStatement);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Statement> getThenStatement() const;
    std::optional<std::shared_ptr<Statement>> getElseOptStatement() const;

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenStatement;
    std::optional<std::shared_ptr<Statement>> elseOptStatement;
};

class CompoundStatement : public Statement {
  public:
    CompoundStatement(std::shared_ptr<Block> block);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Block> getBlock() const;

  private:
    std::shared_ptr<Block> block;
};

class BreakStatement : public Statement {
  public:
    BreakStatement();
    void accept(Visitor &visitor) override;
    std::string getLabel() const;
    void setLabel(const std::string &label);

  private:
    std::string label;
};

class ContinueStatement : public Statement {
  public:
    ContinueStatement();
    void accept(Visitor &visitor) override;
    std::string getLabel() const;
    void setLabel(const std::string &label);

  private:
    std::string label;
};

class WhileStatement : public Statement {
  public:
    WhileStatement(std::shared_ptr<Expression> condition,
                   std::shared_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Statement> getBody() const;
    std::string getLabel() const;
    void setLabel(const std::string &label);

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
    std::string label;
};

class DoWhileStatement : public Statement {
  public:
    DoWhileStatement(std::shared_ptr<Expression> condition,
                     std::shared_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Expression> getCondition() const;
    std::shared_ptr<Statement> getBody() const;
    std::string getLabel() const;
    void setLabel(const std::string &label);

  private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
    std::string label;
};

class ForStatement : public Statement {
  public:
    ForStatement(std::shared_ptr<ForInit> forInit,
                 std::optional<std::shared_ptr<Expression>> condition,
                 std::optional<std::shared_ptr<Expression>> post,
                 std::shared_ptr<Statement> body);
    void accept(Visitor &visitor) override;
    std::shared_ptr<ForInit> getForInit() const;
    std::optional<std::shared_ptr<Expression>> getOptCondition() const;
    std::optional<std::shared_ptr<Expression>> getOptPost() const;
    std::shared_ptr<Statement> getBody() const;
    std::string getLabel() const;
    void setLabel(const std::string &label);

  private:
    std::shared_ptr<ForInit> forInit;
    std::optional<std::shared_ptr<Expression>> optCondition;
    std::optional<std::shared_ptr<Expression>> optPost;
    std::shared_ptr<Statement> body;
    std::string label;
};

class NullStatement : public Statement {
  public:
    NullStatement() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STATEMENT_H
