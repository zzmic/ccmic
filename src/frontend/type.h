#ifndef FRONTEND_TYPES_H
#define FRONTEND_TYPES_H

#include "ast.h"
#include "visitor.h"
#include <memory>
#include <vector>

namespace AST {
/**
 * Base class for types in the AST.
 */
class Type : public AST {
  public:
    /**
     * Default constructor for the type class.
     */
    constexpr Type() = default;

    /**
     * Default destructor for the type class.
     */
    ~Type() override = default;

    /**
     * Delete the copy constructor for the type class.
     */
    constexpr Type(const Type &) = delete;

    /**
     * Delete the copy assignment operator for the type class.
     */
    constexpr Type &operator=(const Type &) = delete;

    /**
     * Default move constructor for the type class.
     */
    constexpr Type(Type &&) = default;

    /**
     * Default move assignment operator for the type class.
     */
    constexpr Type &operator=(Type &&) = default;

    /**
     * Virtual function to check if two types are equal.
     *
     * @param other The other type to compare with.
     * @return True if the types are equal, false otherwise.
     */
    [[nodiscard]] virtual bool isEqual(const Type &other) const;

    /**
     * Overload the equality operator.
     *
     * @param lhs The left-hand side type.
     * @param rhs The right-hand side type.
     * @return True if the types are equal, false otherwise.
     */
    friend bool operator==(const Type &lhs, const Type &rhs);

    /**
     * Overload the inequality operator.
     * @param lhs The left-hand side type.
     * @param rhs The right-hand side type.
     * @return True if the types are not equal, false otherwise.
     */
    friend bool operator!=(const Type &lhs, const Type &rhs);
};

/**
 * Class representing the int type in the AST.
 */
class IntType : public Type {
  public:
    void accept(Visitor &visitor) override;

    /**
     * Overriden `isEqual` function to check if the other type is an int
     * type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is an `IntType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

/**
 * Class representing the long type in the AST.
 */
class LongType : public Type {
  public:
    void accept(Visitor &visitor) override;

    /**
     * Overriden `isEqual` function to check if the other type is a
     * long type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is a `LongType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

/**
 * Class representing the unsigned int type in the AST.
 */
class UIntType : public Type {
  public:
    void accept(Visitor &visitor) override;

    /**
     * Overriden `isEqual` function to check if the other type is an
     * unsigned int type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is a `UIntType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

/**
 * Class representing the unsigned long type in the AST.
 */
class ULongType : public Type {
  public:
    void accept(Visitor &visitor) override;

    /**
     * Overriden `isEqual` function to check if the other type is an
     * unsigned long type.
     *
     * @param other The other type to compare with.
     * @return True if the other type is a `ULongType`, false otherwise.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override;
};

/**
 * Class representing the function type in the AST.
 */
class FunctionType : public Type {
  public:
    /**
     * Constructor for the function-type class.
     *
     * @param parameterTypes The parameter types of the function.
     * @param returnType The return type of the function.
     */
    explicit FunctionType(
        std::unique_ptr<std::vector<std::unique_ptr<Type>>> parameterTypes,
        std::unique_ptr<Type> returnType);

    void accept(Visitor &visitor) override;

    /**
     * Overriden `isEqual` function to check if the other type is a
     * function type with the same parameter and return types.
     */
    [[nodiscard]] bool isEqual(const Type &other) const override;

    [[nodiscard]] const std::vector<std::unique_ptr<Type>> &
    getParameterTypes() const;

    [[nodiscard]] const Type &getReturnType() const;

    Type &getReturnType();

  private:
    /**
     * Parameter types of the function.
     */
    std::unique_ptr<std::vector<std::unique_ptr<Type>>> parameterTypes;

    /**
     * Return type of the function.
     */
    std::unique_ptr<Type> returnType;
};
} // namespace AST

#endif // FRONTEND_TYPES_H
