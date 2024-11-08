

#ifndef FUNCTION_H
#define FUNCTION_H

#include "ast.h"
#include "statement.h"
#include <memory>
#include <string>

namespace AST {

class Function : public AST {
  public:
    Function(const std::string &name, std::shared_ptr<Statement> body);
    void accept(Visitor &vistor) override;
    std::string getName() const;
    std::shared_ptr<Statement> getBody() const;

  private:
    std::string name_;
    std::shared_ptr<Statement> body_;
};
} // Namespace AST

#endif // FUNCTION_H
