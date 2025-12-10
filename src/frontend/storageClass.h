#ifndef FRONTEND_STORAGE_CLASS_H
#define FRONTEND_STORAGE_CLASS_H

#include "ast.h"

namespace AST {
/**
 * Base class for storage classes in the AST.
 */
class StorageClass : public AST {
  public:
    /**
     * Default constructor of the storage-class class.
     */
    constexpr StorageClass() = default;
};

/**
 * Class representing the static storage class in the AST.
 */
class StaticStorageClass : public StorageClass {
  public:
    /**
     * Default constructor of the static-storage-class class.
     */
    constexpr StaticStorageClass() = default;

    void accept(Visitor &visitor) override;
};

/**
 * Class representing the extern storage class in the AST.
 */
class ExternStorageClass : public StorageClass {
  public:
    /**
     * Default constructor of the extern-storage-class class.
     */
    constexpr ExternStorageClass() = default;

    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STORAGE_CLASS_H
