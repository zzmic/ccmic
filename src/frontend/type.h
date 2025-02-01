#ifndef FRONTEND_TYPES_H
#define FRONTEND_TYPES_H

#include <memory>
#include <typeinfo>
#include <vector>

namespace AST {
class Type : public AST {
  public:
    virtual ~Type() = default;
    // Virtual function to check if two types are equal.
    virtual bool isEqual(const Type &other) const {
        return typeid(*this) == typeid(other);
    }
    // Overload the equality operator.
    friend bool operator==(const Type &lhs, const Type &rhs) {
        return lhs.isEqual(rhs);
    }
    // Overload the inequality operator.
    friend bool operator!=(const Type &lhs, const Type &rhs) {
        return !lhs.isEqual(rhs);
    }
};

class IntType : public Type {
  public:
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is an
    // `IntType`.
    bool isEqual(const Type &other) const override {
        return dynamic_cast<const IntType *>(&other) != nullptr;
    }
};

class LongType : public Type {
  public:
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is a
    // `LongType`.
    bool isEqual(const Type &other) const override {
        return dynamic_cast<const LongType *>(&other) != nullptr;
    }
};

class FunctionType : public Type {
  public:
    FunctionType(std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameters,
                 std::shared_ptr<Type> returnType)
        : parameters(parameters), returnType(returnType) {}
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is a
    // `FunctionType`.
    bool isEqual(const Type &other) const override {
        // Dynamically cast the other type to a `FunctionType`.
        const auto *otherFun = dynamic_cast<const FunctionType *>(&other);
        // Return true if the other type is a `FunctionType` (i.e., the casted
        // type is not `nullptr`) and
        return otherFun != nullptr
               // the parameters are equal and
               && *parameters == *otherFun->parameters
               // the return type is equal.
               && *returnType == *otherFun->returnType;
    }
    std::shared_ptr<std::vector<std::shared_ptr<Type>>> getParameters() {
        return parameters;
    }
    std::shared_ptr<Type> getReturnType() { return returnType; }

  private:
    std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameters;
    std::shared_ptr<Type> returnType;
};
} // namespace AST

#endif // FRONTEND_TYPES_H
