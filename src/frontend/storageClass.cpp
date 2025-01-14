#include "storageClass.h"

namespace AST {
void StaticStorageClass::accept(Visitor &visitor) { visitor.visit(*this); }

void ExternStorageClass::accept(Visitor &visitor) { visitor.visit(*this); }
} // Namespace AST
