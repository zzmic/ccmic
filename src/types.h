#ifndef TYPES_H
#define TYPES_H

#include <typeinfo>

class Type {
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
    IntType() {}
    // Override the `isEqual` function to check if the other type is an
    // `IntType`.
    bool isEqual(const Type &other) const override {
        return dynamic_cast<const IntType *>(&other) != nullptr;
    }
};

class FunctionType : public Type {
  public:
    FunctionType(int numParameters) : numParameters(numParameters) {}
    // Override the `isEqual` function to check if the other type is a
    // `FunctionType`.
    bool isEqual(const Type &other) const override {
        // Dynamically cast the other type to a `FunctionType`.
        const auto *otherFunc = dynamic_cast<const FunctionType *>(&other);
        // Return true if the other type is a `FunctionType` (i.e., the casted
        // type is not `nullptr`) and the number of parameters is the same.
        return otherFunc != nullptr &&
               this->numParameters == otherFunc->numParameters;
    }

  private:
    int numParameters;
};

#endif // TYPES_H
