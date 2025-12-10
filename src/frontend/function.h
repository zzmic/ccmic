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
/**
 * Class representing a function in the AST.
 */
class Function : public AST {
  public:
    /**
     * Constructor for the function class.
     *
     * @param identifier The identifier of the function.
     * @param body The body block of the function.
     */
    explicit Function(std::string_view identifier, std::shared_ptr<Block> body);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::string &getIdentifier() const;

    [[nodiscard]] std::shared_ptr<Block> getBody() const;

    void setBody(std::shared_ptr<Block> body);

  private:
    /**
     * The identifier of the function.
     */
    std::string identifier;

    /**
     * The body block of the function.
     */
    std::shared_ptr<Block> body;
};
} // Namespace AST

#endif // FRONTEND_FUNCTION_H
