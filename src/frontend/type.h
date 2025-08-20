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
    [[nodiscard]] virtual bool isEqual(const Type &other) const;
    // Overload the equality operator.
    friend bool operator==(const Type &lhs, const Type &rhs);
    // Overload the inequality operator.
    friend bool operator!=(const Type &lhs, const Type &rhs);
};

class IntType : public Type {
  public:
    constexpr IntType() = default;
    void accept(Visitor &visitor) override;
    // Override the `isEqual` function to check if the other type is an
    // `IntType`.
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

class LongType : public Type {
  public:
    constexpr LongType() = default;
    void accept(Visitor &visitor) override;
    // Override the `isEqual` function to check if the other type is a
    // `LongType`.
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

class FunctionType : public Type {
  public:
    explicit FunctionType(std::vector<std::unique_ptr<Type>> parameterTypes,
                          std::unique_ptr<Type> returnType)
        : parameterTypes(std::move(parameterTypes)),
          returnType(std::move(returnType)) {}
    void accept(Visitor &visitor) override;
    // Override the `isEqual` function to check if the other type is a
    // `FunctionType`.
    [[nodiscard]] bool isEqual(const Type &other) const override;
    [[nodiscard]] std::vector<std::unique_ptr<Type>> &getParameterTypes();
    [[nodiscard]] std::unique_ptr<Type> &getReturnType();

  private:
    std::vector<std::unique_ptr<Type>> parameterTypes;
    std::unique_ptr<Type> returnType;
};
} // namespace AST

#endif // FRONTEND_TYPES_H
