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
     * Default constructor for the AST class.
     */
    constexpr AST() = default;

    /**
     * Default virtual destructor for the AST class.
     */
    virtual ~AST() = default;

    /**
     * Delete the copy constructor for the AST class.
     */
    constexpr AST(const AST &) = delete;

    /**
     * Delete the copy assignment operator for the AST class.
     */
    constexpr AST &operator=(const AST &) = delete;

    /**
     * Default move constructor for the AST class.
     */
    constexpr AST(AST &&) = default;

    /**
     * Default move assignment operator for the AST class.
     */
    constexpr AST &operator=(AST &&) = default;

    /**
     * Pure virtual method to accept a visitor.
     *
     * @param visitor The visitor to accept.
     */
    virtual void accept(Visitor &visitor) = 0;
};
} // Namespace AST

#endif // FRONTEND_AST_H
