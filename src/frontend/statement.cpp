#include "statement.h"
#include "block.h"
#include "forInit.h"
#include "visitor.h"

namespace AST {
ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}

void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *ReturnStatement::getExpression() const { return expr.get(); }

void ReturnStatement::setExpression(std::unique_ptr<Expression> newExpr) {
    this->expr = std::move(newExpr);
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}

void ExpressionStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *ExpressionStatement::getExpression() const { return expr.get(); }

IfStatement::IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement,
                         std::unique_ptr<Statement> elseOptStatement)
    : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
      elseOptStatement(std::move(elseOptStatement)) {}

IfStatement::IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement)
    : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
      elseOptStatement(nullptr) {}

void IfStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *IfStatement::getCondition() const { return condition.get(); }

Statement *IfStatement::getThenStatement() const { return thenStatement.get(); }

Statement *IfStatement::getElseOptStatement() const {
    return elseOptStatement.get();
}

CompoundStatement::CompoundStatement(std::unique_ptr<Block> block)
    : block(std::move(block)) {}

CompoundStatement::~CompoundStatement() = default;

void CompoundStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Block *CompoundStatement::getBlock() const { return block.get(); }

void BreakStatement::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &BreakStatement::getLabel() const { return label; }

void BreakStatement::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

void ContinueStatement::accept(Visitor &visitor) { visitor.visit(*this); }

const std::string &ContinueStatement::getLabel() const { return label; }

void ContinueStatement::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

WhileStatement::WhileStatement(std::unique_ptr<Expression> condition,
                               std::unique_ptr<Statement> body)
    : condition(std::move(condition)), body(std::move(body)) {}

void WhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *WhileStatement::getCondition() const { return condition.get(); }

Statement *WhileStatement::getBody() const { return body.get(); }

const std::string &WhileStatement::getLabel() const { return label; }

void WhileStatement::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

DoWhileStatement::DoWhileStatement(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Statement> body)
    : condition(std::move(condition)), body(std::move(body)) {}

void DoWhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Expression *DoWhileStatement::getCondition() const { return condition.get(); }

Statement *DoWhileStatement::getBody() const { return body.get(); }

const std::string &DoWhileStatement::getLabel() const { return label; }

void DoWhileStatement::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

ForStatement::ForStatement(std::unique_ptr<ForInit> forInit,
                           std::unique_ptr<Expression> condition,
                           std::unique_ptr<Expression> post,
                           std::unique_ptr<Statement> body)
    : forInit(std::move(forInit)), optCondition(std::move(condition)),
      optPost(std::move(post)), body(std::move(body)) {}

ForStatement::~ForStatement() = default;

void ForStatement::accept(Visitor &visitor) { visitor.visit(*this); }

ForInit *ForStatement::getForInit() const { return forInit.get(); }

Expression *ForStatement::getOptCondition() const { return optCondition.get(); }

Expression *ForStatement::getOptPost() const { return optPost.get(); }

Statement *ForStatement::getBody() const { return body.get(); }

const std::string &ForStatement::getLabel() const { return label; }

void ForStatement::setLabel(std::string_view newLabel) {
    this->label = newLabel;
}

void NullStatement::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
