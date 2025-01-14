#ifndef FRONTEND_STORAGE_CLASS_H
#define FRONTEND_STORAGE_CLASS_H

#include "ast.h"

namespace AST {
class StorageClass : public AST {};

class StaticStorageClass : public StorageClass {
  public:
    void accept(Visitor &visitor) override;
};

class ExternStorageClass : public StorageClass {
  public:
    void accept(Visitor &visitor) override;
};
} // Namespace AST

#endif // FRONTEND_STORAGE_CLASS_H
