#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "ast.h"
#include "function.h"
#include <memory>

namespace AST {
/**
 * Class representing the entire program in the AST.
 *
 * The `Program` class serves as the root node of the Abstract Syntax Tree (AST)
 * and encapsulates a collection of declarations that make up the program.
 */
class Program : public AST {
  public:
    /**
     * Constructor for the `Program` class.
     *
     * @param declaration The list of declarations in the program.
     */
    explicit Program(std::unique_ptr<std::vector<std::unique_ptr<Declaration>>>
                         declarations);

    void accept(Visitor &visitor) override;

    [[nodiscard]] const std::vector<std::unique_ptr<Declaration>> &
    getDeclarations() const;

    void
    setDeclarations(std::unique_ptr<std::vector<std::unique_ptr<Declaration>>>
                        declarations);

  private:
    /**
     * The list of declarations that make up the program.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Declaration>>> declarations;
};
} // Namespace AST

#endif // FRONTEND_PROGRAM_H
