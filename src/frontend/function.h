#ifndef FRONTEND_FUNCTION_H
#define FRONTEND_FUNCTION_H

#include "ast.h"
#include "block.h"
#include "statement.h"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace AST {
class Function : public AST {
  public:
    explicit Function(std::string_view identifier, Block *body);
    ~Function(); // Need destructor to clean up raw pointer
    void accept(Visitor &vistor) override;
    [[nodiscard]] std::string &getIdentifier();
    [[nodiscard]] Block *getBody();
    void setBody(Block *body);

  private:
    std::string identifier;
    Block *body;
};
} // Namespace AST

#endif // FRONTEND_FUNCTION_H
