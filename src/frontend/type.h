#ifndef FRONTEND_TYPES_H
#define FRONTEND_TYPES_H

#include "ast.h"
#include <memory>
#include <typeinfo>
#include <vector>

namespace AST {
/**
 * Base class for types in the AST.
 */
class Type : public AST {
  public:
    /**
     * Default constructor of the type class.
     */
    constexpr Type() = default;
    /**
     * Default virtual destructor for the type class.
     */
    virtual ~Type() = default;
    /**
     * Virtual function to check if two types are equal.
     *
     * @param other The other type to compare with.
     * @return True if the types are equal, false otherwise.
     */
    [[nodiscard]] virtual bool isEqual(const Type &other) const {
        return typeid(*this) == typeid(other);
    }

    /**
     * Overload the equality operator.
     *
     * @param lhs The left-hand side type.
     * @param rhs The right-hand side type.
     * @return True if the types are equal, false otherwise.
     */
    [[nodiscard]] friend bool operator==(const Type &lhs, const Type &rhs) {
        return lhs.isEqual(rhs);
    }

    /**
     * Overload the inequality operator.
     * @param lhs The left-hand side type.
     * @param rhs The right-hand side type.
     * @return True if the types are not equal, false otherwise.
     */
    [[nodiscard]] friend bool operator!=(const Type &lhs, const Type &rhs) {
        return !lhs.isEqual(rhs);
    }
};

/**
 * Class representing the int type in the AST.
 */
class IntType : public Type {
  public:
    /**
     * Default constructor of the int-type class.
     */
    constexpr IntType() = default;

    void accept(Visitor &visitor) override { visitor.visit(*this); }

    /**
     * Overriden `isEqual` function to check if the other type is an int
     * type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is an `IntType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override {
        return dynamic_cast<const IntType *>(&other) != nullptr;
    }
};

/**
 * Class representing the long type in the AST.
 */
class LongType : public Type {
  public:
    /**
     * Default constructor of the long-type class.
     */
    constexpr LongType() = default;

    void accept(Visitor &visitor) override { visitor.visit(*this); }
    /**
     * Overriden `isEqual` function to check if the other type is a
     * long type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is a `LongType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override {
        return dynamic_cast<const LongType *>(&other) != nullptr;
    }
};

/**
 * Class representing the function type in the AST.
 */
class FunctionType : public Type {
  public:
    /**
     * Constructor of the function-type class.
     *
     * @param parameterTypes The parameter types of the function.
     * @param returnType The return type of the function.
     */
    explicit FunctionType(
        std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameterTypes,
        std::shared_ptr<Type> returnType)
        : parameterTypes(parameterTypes), returnType(returnType) {}

    void accept(Visitor &visitor) override { visitor.visit(*this); }

    /**
     * Overriden `isEqual` function to check if the other type is a
     * function type with the same parameter and return types.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override {
        const auto *otherFn = dynamic_cast<const FunctionType *>(&other);
        if (!otherFn) {
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
    /**
     * Parameter types of the function.
     */
    std::shared_ptr<std::vector<std::shared_ptr<Type>>> parameterTypes;

    /**
     * Return type of the function.
     */
    std::shared_ptr<Type> returnType;
};
} // namespace AST

#endif // FRONTEND_TYPES_H
