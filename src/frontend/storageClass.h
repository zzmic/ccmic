#ifndef FRONTEND_STORAGE_CLASS_H
#define FRONTEND_STORAGE_CLASS_H

#include "ast.h"

namespace AST {
class StorageClass : public AST {
  public:
    constexpr StorageClass() = default;
};

class StaticStorageClass : public StorageClass {
  public:
    constexpr StaticStorageClass() = default;
    void accept(Visitor &visitor) override;
};

class ExternStorageClass : public StorageClass {
  public:
    constexpr ExternStorageClass() = default;
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STORAGE_CLASS_H
