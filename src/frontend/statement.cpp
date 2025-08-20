#include "statement.h"
#include "block.h"
#include "visitor.h"

namespace AST {
ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}

void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &ReturnStatement::getExpression() { return expr; }

void ReturnStatement::setExpression(std::unique_ptr<Expression> newExpr) {
    this->expr = std::move(newExpr);
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expr)
    : expr(std::move(expr)) {}

void ExpressionStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &ExpressionStatement::getExpression() {
    return expr;
}

void ExpressionStatement::setExpression(std::unique_ptr<Expression> newExpr) {
    this->expr = std::move(newExpr);
}

IfStatement::IfStatement(
    std::unique_ptr<Expression> condition,
    std::unique_ptr<Statement> thenStatement,
    std::optional<std::unique_ptr<Statement>> elseOptStatement)
    : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
      elseOptStatement(std::move(elseOptStatement)) {}

IfStatement::IfStatement(std::unique_ptr<Expression> condition,
                         std::unique_ptr<Statement> thenStatement)
    : condition(std::move(condition)), thenStatement(std::move(thenStatement)) {
}

void IfStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &IfStatement::getCondition() { return condition; }

std::unique_ptr<Statement> &IfStatement::getThenStatement() {
    return thenStatement;
}

std::optional<std::unique_ptr<Statement>> &IfStatement::getElseOptStatement() {
    return elseOptStatement;
}

void IfStatement::setCondition(std::unique_ptr<Expression> newCondition) {
    this->condition = std::move(newCondition);
}

void IfStatement::setThenStatement(
    std::unique_ptr<Statement> newThenStatement) {
    this->thenStatement = std::move(newThenStatement);
}

void IfStatement::setElseOptStatement(
    std::optional<std::unique_ptr<Statement>> newElseOptStatement) {
    this->elseOptStatement = std::move(newElseOptStatement);
}

CompoundStatement::CompoundStatement(Block *block) : block(block) {}

CompoundStatement::~CompoundStatement() {
    // Clean up the raw pointer.
    delete block;
}

void CompoundStatement::accept(Visitor &visitor) { visitor.visit(*this); }

Block *CompoundStatement::getBlock() { return block; }

void CompoundStatement::setBlock(Block *newBlock) { this->block = newBlock; }

void BreakStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &BreakStatement::getLabel() { return label; }

void BreakStatement::setLabel(std::string_view newLabel) {
    this->label = std::string(newLabel);
}

void ContinueStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::string &ContinueStatement::getLabel() { return label; }

void ContinueStatement::setLabel(std::string_view newLabel) {
    this->label = std::string(newLabel);
}

WhileStatement::WhileStatement(std::unique_ptr<Expression> condition,
                               std::unique_ptr<Statement> body)
    : condition(std::move(condition)), body(std::move(body)) {}

void WhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &WhileStatement::getCondition() {
    return condition;
}

std::unique_ptr<Statement> &WhileStatement::getBody() { return body; }

std::string &WhileStatement::getLabel() { return label; }

void WhileStatement::setCondition(std::unique_ptr<Expression> newCondition) {
    this->condition = std::move(newCondition);
}

void WhileStatement::setBody(std::unique_ptr<Statement> newBody) {
    this->body = std::move(newBody);
}

void WhileStatement::setLabel(std::string_view newLabel) {
    this->label = std::string(newLabel);
}

DoWhileStatement::DoWhileStatement(std::unique_ptr<Expression> condition,
                                   std::unique_ptr<Statement> body)
    : condition(std::move(condition)), body(std::move(body)) {}

void DoWhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<Expression> &DoWhileStatement::getCondition() {
    return condition;
}

std::unique_ptr<Statement> &DoWhileStatement::getBody() { return body; }

std::string &DoWhileStatement::getLabel() { return label; }

void DoWhileStatement::setCondition(std::unique_ptr<Expression> newCondition) {
    this->condition = std::move(newCondition);
}

void DoWhileStatement::setBody(std::unique_ptr<Statement> newBody) {
    this->body = std::move(newBody);
}

void DoWhileStatement::setLabel(std::string_view newLabel) {
    this->label = std::string(newLabel);
}

ForStatement::ForStatement(std::unique_ptr<ForInit> forInit,
                           std::optional<std::unique_ptr<Expression>> condition,
                           std::optional<std::unique_ptr<Expression>> post,
                           std::unique_ptr<Statement> body)
    : forInit(std::move(forInit)), optCondition(std::move(condition)),
      optPost(std::move(post)), body(std::move(body)) {}

void ForStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::unique_ptr<ForInit> &ForStatement::getForInit() { return forInit; }

std::optional<std::unique_ptr<Expression>> &ForStatement::getOptCondition() {
    return optCondition;
}

std::optional<std::unique_ptr<Expression>> &ForStatement::getOptPost() {
    return optPost;
}

std::unique_ptr<Statement> &ForStatement::getBody() { return body; }

std::string &ForStatement::getLabel() { return label; }

void ForStatement::setForInit(std::unique_ptr<ForInit> newForInit) {
    this->forInit = std::move(newForInit);
}

void ForStatement::setOptCondition(
    std::optional<std::unique_ptr<Expression>> newOptCondition) {
    this->optCondition = std::move(newOptCondition);
}

void ForStatement::setOptPost(
    std::optional<std::unique_ptr<Expression>> newOptPost) {
    this->optPost = std::move(newOptPost);
}

void ForStatement::setBody(std::unique_ptr<Statement> newBody) {
    this->body = std::move(newBody);
}

void ForStatement::setLabel(std::string_view newLabel) {
    this->label = std::string(newLabel);
}

void NullStatement::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
