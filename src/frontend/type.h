#ifndef FRONTEND_TYPES_H
#define FRONTEND_TYPES_H

#include "ast.h"
#include <memory>
#include <typeinfo>
#include <vector>

namespace AST {
class Type : public AST {
  public:
    constexpr Type() = default;
    // Virtual function to check if two types are equal.
    [[nodiscard]] virtual bool isEqual(const Type &other) const {
        return typeid(*this) == typeid(other);
    }
    // Overload the equality operator.
    [[nodiscard]] friend bool operator==(const Type &lhs, const Type &rhs) {
        return lhs.isEqual(rhs);
    }
    // Overload the inequality operator.
    [[nodiscard]] friend bool operator!=(const Type &lhs, const Type &rhs) {
        return !lhs.isEqual(rhs);
    }
};

class IntType : public Type {
  public:
    constexpr IntType() = default;
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is an
    // `IntType`.
    [[nodiscard]] bool isEqual(const Type &other) const override {
        return dynamic_cast<const IntType *>(&other) != nullptr;
    }
};

class LongType : public Type {
  public:
    constexpr LongType() = default;
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is a
    // `LongType`.
    [[nodiscard]] bool isEqual(const Type &other) const override {
        return dynamic_cast<const LongType *>(&other) != nullptr;
    }
};

class FunctionType : public Type {
  public:
    explicit FunctionType(
        std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameterTypes,
        std::shared_ptr<Type> returnType)
        : parameterTypes(parameterTypes), returnType(returnType) {}
    void accept(Visitor &visitor) override { visitor.visit(*this); }
    // Override the `isEqual` function to check if the other type is a
    // `FunctionType`.
    [[nodiscard]] bool isEqual(const Type &other) const override {
        const auto *otherFn = dynamic_cast<const FunctionType *>(&other);
        if (otherFn == nullptr) {
            return false;
        }
        // Compare parameter types by value.
        const auto &params1 = *parameterTypes;
        const auto &params2 = *otherFn->parameterTypes;
        if (params1.size() != params2.size()) {
            return false;
        }
        for (size_t i = 0; i < params1.size(); ++i) {
            if (*(params1[i]) != *(params2[i])) {
                return false;
            }
        }
        // Compare return types by value.
        return *returnType == *otherFn->returnType;
    }
    [[nodiscard]] const std::shared_ptr<std::vector<std::shared_ptr<Type>>> &
    getParameterTypes() const {
        return parameterTypes;
    }
    [[nodiscard]] std::shared_ptr<Type> getReturnType() const {
        return returnType;
    }

  private:
    std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameterTypes;
    std::shared_ptr<Type> returnType;
};
} // namespace AST

#endif // FRONTEND_TYPES_H
