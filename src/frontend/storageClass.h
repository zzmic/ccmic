#ifndef FRONTEND_STORAGE_CLASS_H
#define FRONTEND_STORAGE_CLASS_H

#include "ast.h"

namespace AST {
/**
 * Base class for storage classes in the AST.
 */
class StorageClass : public AST {};

/**
 * Class representing the static storage class in the AST.
 */
class StaticStorageClass : public StorageClass {
  public:
    void accept(Visitor &visitor) override;
};

/**
 * Class representing the extern storage class in the AST.
 */
class ExternStorageClass : public StorageClass {
  public:
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STORAGE_CLASS_H
