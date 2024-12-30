#ifndef FUNCTION_H
#define FUNCTION_H

#include "ast.h"
#include "block.h"
#include "statement.h"
#include <memory>
#include <string>
#include <vector>

namespace AST {
class Function : public AST {
  public:
    Function(const std::string &name, std::shared_ptr<Block> body);
    void accept(Visitor &vistor) override;
    std::string getName() const;
    std::shared_ptr<Block> getBody() const;

  private:
    std::string name;
    std::shared_ptr<Block> body;
};
} // Namespace AST

#endif // FUNCTION_H
