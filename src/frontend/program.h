#ifndef FRONTEND_PROGRAM_H
#define FRONTEND_PROGRAM_H

#include "ast.h"
#include "function.h"
#include <memory>

namespace AST {
class Program : public AST {
  public:
    explicit Program(std::shared_ptr<std::vector<std::shared_ptr<Declaration>>>
                         declarations);
    void accept(Visitor &visitor) override;
    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<Declaration>>> &
    getDeclarations() const;
    void
    setDeclarations(std::shared_ptr<std::vector<std::shared_ptr<Declaration>>>
                        declarations);

  private:
    std::shared_ptr<std::vector<std::shared_ptr<Declaration>>> declarations;
};
} // Namespace AST

#endif // FRONTEND_PROGRAM_H
