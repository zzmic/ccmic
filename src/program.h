

#ifndef PROGRAM_H
#define PROGRAM_H

#include "ast.h"
#include "function.h"
#include <memory>

namespace AST {
class Program : public AST {
  public:
    Program(std::shared_ptr<Function> function);
    void accept(Visitor &visitor) override;
    std::shared_ptr<Function> getFunction() const;

  private:
    std::shared_ptr<Function> function_;
};
} // Namespace AST

#endif // PROGRAM_H
