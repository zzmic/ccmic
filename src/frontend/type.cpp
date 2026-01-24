#include "type.h"
#include <stdexcept>

namespace AST {
bool Type::isEqual(const Type &other) const {
    return typeid(*this) == typeid(other);
}

bool operator==(const Type &lhs, const Type &rhs) { return lhs.isEqual(rhs); }

bool operator!=(const Type &lhs, const Type &rhs) { return !lhs.isEqual(rhs); }

void IntType::accept(Visitor &visitor) { visitor.visit(*this); }

bool IntType::isEqual(const Type &other) const {
    return dynamic_cast<const IntType *>(&other) != nullptr;
}

void LongType::accept(Visitor &visitor) { visitor.visit(*this); }

bool LongType::isEqual(const Type &other) const {
    return dynamic_cast<const LongType *>(&other) != nullptr;
}

void UIntType::accept(Visitor &visitor) { visitor.visit(*this); }

bool UIntType::isEqual(const Type &other) const {
    return dynamic_cast<const UIntType *>(&other) != nullptr;
}

void ULongType::accept(Visitor &visitor) { visitor.visit(*this); }

bool ULongType::isEqual(const Type &other) const {
    return dynamic_cast<const ULongType *>(&other) != nullptr;
}

FunctionType::FunctionType(
    std::unique_ptr<std::vector<std::unique_ptr<Type>>> parameterTypes,
    std::unique_ptr<Type> returnType)
    : parameterTypes(std::move(parameterTypes)),
      returnType(std::move(returnType)) {
    if (!this->parameterTypes) {
        throw std::invalid_argument(
            "Creating FunctionType with null parameterTypes in FunctionType");
    }
    if (!this->returnType) {
        throw std::invalid_argument(
            "Creating FunctionType with null returnType in FunctionType");
    }
}

void FunctionType::accept(Visitor &visitor) { visitor.visit(*this); }

bool FunctionType::isEqual(const Type &other) const {
    const auto *otherFn = dynamic_cast<const FunctionType *>(&other);
    if (!otherFn) {
        return false;
    }
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
    return *returnType == *otherFn->returnType;
}

const std::vector<std::unique_ptr<Type>> &
FunctionType::getParameterTypes() const {
    return *parameterTypes;
}

const Type &FunctionType::getReturnType() const { return *returnType; }
} // namespace AST
