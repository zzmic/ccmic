#ifndef FRONTEND_FUNCTION_H
#define FRONTEND_FUNCTION_H

#include "ast.h"
#include "block.h"
#include "statement.h"
#include <memory>
#include <string>
#include <vector>

namespace AST {
class Function : public AST {
  public:
    Function(const std::string &identifier, std::shared_ptr<Block> body);
    void accept(Visitor &vistor) override;
    std::string getIdentifier() const;
    std::shared_ptr<Block> getBody() const;
    void setBody(std::shared_ptr<Block> body);

  private:
    std::string identifier;
    std::shared_ptr<Block> body;
};
} // Namespace AST

#endif // FRONTEND_FUNCTION_H
