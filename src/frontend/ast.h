#ifndef FRONTEND_AST_H
#define FRONTEND_AST_H

#include "visitor.h"

namespace AST {
/**
 * Base class for all AST nodes.
 */
class AST {
  public:
    /**
     * Default virtual destructor for AST nodes.
     */
    virtual ~AST() = default;

    /**
     * Pure virtual method to accept a visitor.
     *
     * @param visitor The visitor to accept.
     */
    virtual void accept(Visitor &visitor) = 0;
};
} // Namespace AST

#endif // FRONTEND_AST_H
