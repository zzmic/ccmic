#include "statement.h"
#include "visitor.h"

namespace AST {
ReturnStatement::ReturnStatement(std::shared_ptr<Expression> expr)
    : expr(expr) {}

void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ReturnStatement::getExpression() const {
    return expr;
}

ExpressionStatement::ExpressionStatement(std::shared_ptr<Expression> expr)
    : expr(expr) {}

void ExpressionStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> ExpressionStatement::getExpression() const {
    return expr;
}

IfStatement::IfStatement(
    std::shared_ptr<Expression> condition,
    std::shared_ptr<Statement> thenStatement,
    std::optional<std::shared_ptr<Statement>> elseOptStatement)
    : condition(condition), thenStatement(thenStatement),
      elseOptStatement(elseOptStatement) {}

IfStatement::IfStatement(std::shared_ptr<Expression> condition,
                         std::shared_ptr<Statement> thenStatement)
    : condition(condition), thenStatement(thenStatement) {}

void IfStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> IfStatement::getCondition() const {
    return condition;
}

std::shared_ptr<Statement> IfStatement::getThenStatement() const {
    return thenStatement;
}

std::optional<std::shared_ptr<Statement>>
IfStatement::getElseOptStatement() const {
    return elseOptStatement;
}

CompoundStatement::CompoundStatement(std::shared_ptr<Block> block)
    : block(block) {}

void CompoundStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Block> CompoundStatement::getBlock() const { return block; }

BreakStatement::BreakStatement() {
    label = "break" + std::to_string(counter++);
}

void BreakStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::string BreakStatement::getLabel() const { return label; }

ContinueStatement::ContinueStatement() {
    label = "continue" + std::to_string(counter++);
}

void ContinueStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::string ContinueStatement::getLabel() const { return label; }

WhileStatement::WhileStatement(std::shared_ptr<Expression> condition,
                               std::shared_ptr<Statement> body)
    : condition(condition), body(body) {
    label = "while" + std::to_string(counter++);
}

void WhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> WhileStatement::getCondition() const {
    return condition;
}

std::shared_ptr<Statement> WhileStatement::getBody() const { return body; }

std::string WhileStatement::getLabel() const { return label; }

DoWhileStatement::DoWhileStatement(std::shared_ptr<Expression> condition,
                                   std::shared_ptr<Statement> body)
    : condition(condition), body(body) {
    label = "doWhile" + std::to_string(counter++);
}

void DoWhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<Expression> DoWhileStatement::getCondition() const {
    return condition;
}

std::shared_ptr<Statement> DoWhileStatement::getBody() const { return body; }

std::string DoWhileStatement::getLabel() const { return label; }

ForStatement::ForStatement(std::shared_ptr<ForInit> forInit,
                           std::optional<std::shared_ptr<Expression>> condition,
                           std::optional<std::shared_ptr<Expression>> post,
                           std::shared_ptr<Statement> body)
    : forInit(forInit), optCondition(condition), optPost(post), body(body) {
    label = "for" + std::to_string(counter++);
}

void ForStatement::accept(Visitor &visitor) { visitor.visit(*this); }

std::shared_ptr<ForInit> ForStatement::getForInit() const { return forInit; }

std::optional<std::shared_ptr<Expression>>
ForStatement::getOptCondition() const {
    return optCondition;
}

std::optional<std::shared_ptr<Expression>> ForStatement::getOptPost() const {
    return optPost;
}

std::shared_ptr<Statement> ForStatement::getBody() const { return body; }

std::string ForStatement::getLabel() const { return label; }

void NullStatement::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
