#include "irOptimizationPasses.h"
#include <optional>

/**
 * Unnamed namespace for helper functions for the IR optimization passes.
 */
namespace {
/**
 * Struct representing a constant value, which can be either an int or a long.
 */
struct ConstValue {
    bool isLong;
    long value;
};

/**
 * Retrieve the constant value from an IR value if it is a constant.
 *
 * @param value The IR value to check.
 * @return An optional `ConstValue` if the IR value is a constant, otherwise
 * `std::nullopt`.
 */
std::optional<ConstValue> getConstValue(const IR::Value *value) {
    auto constantValue = dynamic_cast<const IR::ConstantValue *>(value);
    if (!constantValue) {
        return std::nullopt;
    }
    auto astConst = constantValue->getASTConstant();
    if (auto intConst = dynamic_cast<const AST::ConstantInt *>(astConst)) {
        return ConstValue{false, static_cast<long>(intConst->getValue())};
    }
    if (auto longConst = dynamic_cast<const AST::ConstantLong *>(astConst)) {
        return ConstValue{true, longConst->getValue()};
    }
    return std::nullopt;
}

/**
 * Create an IR constant value from a `ConstValue`.
 *
 * @param value The `ConstValue` to convert.
 * @return A `std::unique_ptr` to the created IR constant value.
 */
std::unique_ptr<IR::Value> makeConstValue(const ConstValue &value) {
    if (value.isLong) {
        return std::make_unique<IR::ConstantValue>(
            std::make_unique<AST::ConstantLong>(value.value));
    }
    return std::make_unique<IR::ConstantValue>(
        std::make_unique<AST::ConstantInt>(static_cast<int>(value.value)));
}

/**
 * Attempt to fold a unary operation on a constant value.
 *
 * @param op The unary operator.
 * @param src The source constant value.
 * @return An optional `ConstValue` if the operation can be folded, otherwise
 * `std::nullopt`.
 */
std::optional<ConstValue> foldUnary(const IR::UnaryOperator *op,
                                    const ConstValue &src) {
    if (dynamic_cast<const IR::NegateOperator *>(op)) {
        return ConstValue{src.isLong, -src.value};
    }
    if (dynamic_cast<const IR::ComplementOperator *>(op)) {
        return ConstValue{src.isLong, ~src.value};
    }
    if (dynamic_cast<const IR::NotOperator *>(op)) {
        return ConstValue{false, src.value == 0 ? 1 : 0};
    }
    return std::nullopt;
}

/**
 * Attempt to fold a binary operation on two constant values.
 *
 * @param op The binary operator.
 * @param lhs The left-hand side constant value.
 * @param rhs The right-hand side constant value.
 * @return An optional `ConstValue` if the operation can be folded, otherwise
 * `std::nullopt`.
 */
std::optional<ConstValue> foldBinary(const IR::BinaryOperator *op,
                                     const ConstValue &lhs,
                                     const ConstValue &rhs) {
    const bool isLong = lhs.isLong || rhs.isLong;
    if (dynamic_cast<const IR::AddOperator *>(op)) {
        return ConstValue{isLong, lhs.value + rhs.value};
    }
    if (dynamic_cast<const IR::SubtractOperator *>(op)) {
        return ConstValue{isLong, lhs.value - rhs.value};
    }
    if (dynamic_cast<const IR::MultiplyOperator *>(op)) {
        return ConstValue{isLong, lhs.value * rhs.value};
    }
    if (dynamic_cast<const IR::DivideOperator *>(op)) {
        if (rhs.value == 0) {
            return std::nullopt;
        }
        return ConstValue{isLong, lhs.value / rhs.value};
    }
    if (dynamic_cast<const IR::RemainderOperator *>(op)) {
        if (rhs.value == 0) {
            return std::nullopt;
        }
        return ConstValue{isLong, lhs.value % rhs.value};
    }
    if (dynamic_cast<const IR::EqualOperator *>(op)) {
        return ConstValue{false, lhs.value == rhs.value ? 1 : 0};
    }
    if (dynamic_cast<const IR::NotEqualOperator *>(op)) {
        return ConstValue{false, lhs.value != rhs.value ? 1 : 0};
    }
    if (dynamic_cast<const IR::LessThanOperator *>(op)) {
        return ConstValue{false, lhs.value < rhs.value ? 1 : 0};
    }
    if (dynamic_cast<const IR::LessThanOrEqualOperator *>(op)) {
        return ConstValue{false, lhs.value <= rhs.value ? 1 : 0};
    }
    if (dynamic_cast<const IR::GreaterThanOperator *>(op)) {
        return ConstValue{false, lhs.value > rhs.value ? 1 : 0};
    }
    if (dynamic_cast<const IR::GreaterThanOrEqualOperator *>(op)) {
        return ConstValue{false, lhs.value >= rhs.value ? 1 : 0};
    }
    return std::nullopt;
}

/**
 * Clone an AST constant.
 *
 * @param constant The AST constant to clone.
 * @return A `std::unique_ptr` to the cloned AST constant.
 */
std::unique_ptr<AST::Constant> cloneASTConstant(const AST::Constant *constant) {
    if (auto intConst = dynamic_cast<const AST::ConstantInt *>(constant)) {
        return std::make_unique<AST::ConstantInt>(intConst->getValue());
    }
    if (auto longConst = dynamic_cast<const AST::ConstantLong *>(constant)) {
        return std::make_unique<AST::ConstantLong>(longConst->getValue());
    }
    throw std::logic_error("Unsupported AST constant in cloneASTConstant");
}

/**
 * Clone an IR value.
 *
 * @param value The IR value to clone.
 * @return A `std::unique_ptr` to the cloned IR value.
 */
std::unique_ptr<IR::Value> cloneValue(const IR::Value *value) {
    if (auto constantValue = dynamic_cast<const IR::ConstantValue *>(value)) {
        return std::make_unique<IR::ConstantValue>(
            cloneASTConstant(constantValue->getASTConstant()));
    }
    if (auto variableValue = dynamic_cast<const IR::VariableValue *>(value)) {
        return std::make_unique<IR::VariableValue>(
            variableValue->getIdentifier());
    }
    throw std::logic_error("Unsupported IR value in cloneValue");
}

/**
 * Clone an IR unary operator.
 *
 * @param op The unary operator to clone.
 * @return A `std::unique_ptr` to the cloned unary operator.
 */
std::unique_ptr<IR::UnaryOperator>
cloneUnaryOperator(const IR::UnaryOperator *op) {
    if (dynamic_cast<const IR::NegateOperator *>(op)) {
        return std::make_unique<IR::NegateOperator>();
    }
    if (dynamic_cast<const IR::ComplementOperator *>(op)) {
        return std::make_unique<IR::ComplementOperator>();
    }
    if (dynamic_cast<const IR::NotOperator *>(op)) {
        return std::make_unique<IR::NotOperator>();
    }
    throw std::logic_error("Unsupported unary operator in cloneUnaryOperator");
}

/**
 * Clone an IR binary operator.
 *
 * @param op The binary operator to clone.
 * @return A `std::unique_ptr` to the cloned binary operator.
 */
std::unique_ptr<IR::BinaryOperator>
cloneBinaryOperator(const IR::BinaryOperator *op) {
    if (dynamic_cast<const IR::AddOperator *>(op)) {
        return std::make_unique<IR::AddOperator>();
    }
    if (dynamic_cast<const IR::SubtractOperator *>(op)) {
        return std::make_unique<IR::SubtractOperator>();
    }
    if (dynamic_cast<const IR::MultiplyOperator *>(op)) {
        return std::make_unique<IR::MultiplyOperator>();
    }
    if (dynamic_cast<const IR::DivideOperator *>(op)) {
        return std::make_unique<IR::DivideOperator>();
    }
    if (dynamic_cast<const IR::RemainderOperator *>(op)) {
        return std::make_unique<IR::RemainderOperator>();
    }
    if (dynamic_cast<const IR::EqualOperator *>(op)) {
        return std::make_unique<IR::EqualOperator>();
    }
    if (dynamic_cast<const IR::NotEqualOperator *>(op)) {
        return std::make_unique<IR::NotEqualOperator>();
    }
    if (dynamic_cast<const IR::LessThanOperator *>(op)) {
        return std::make_unique<IR::LessThanOperator>();
    }
    if (dynamic_cast<const IR::LessThanOrEqualOperator *>(op)) {
        return std::make_unique<IR::LessThanOrEqualOperator>();
    }
    if (dynamic_cast<const IR::GreaterThanOperator *>(op)) {
        return std::make_unique<IR::GreaterThanOperator>();
    }
    if (dynamic_cast<const IR::GreaterThanOrEqualOperator *>(op)) {
        return std::make_unique<IR::GreaterThanOrEqualOperator>();
    }
    throw std::logic_error(
        "Unsupported binary operator in cloneBinaryOperator");
}

/**
 * Clone an IR instruction.
 *
 * @param instruction The IR instruction to clone.
 * @return A `std::unique_ptr` to the cloned IR instruction.
 */
std::unique_ptr<IR::Instruction>
cloneInstruction(const IR::Instruction *instruction) {
    if (auto returnInstr =
            dynamic_cast<const IR::ReturnInstruction *>(instruction)) {
        return std::make_unique<IR::ReturnInstruction>(
            cloneValue(returnInstr->getReturnValue()));
    }
    if (auto signExtend =
            dynamic_cast<const IR::SignExtendInstruction *>(instruction)) {
        return std::make_unique<IR::SignExtendInstruction>(
            cloneValue(signExtend->getSrc()), cloneValue(signExtend->getDst()));
    }
    if (auto truncate =
            dynamic_cast<const IR::TruncateInstruction *>(instruction)) {
        return std::make_unique<IR::TruncateInstruction>(
            cloneValue(truncate->getSrc()), cloneValue(truncate->getDst()));
    }
    if (auto unaryInstr =
            dynamic_cast<const IR::UnaryInstruction *>(instruction)) {
        return std::make_unique<IR::UnaryInstruction>(
            cloneUnaryOperator(unaryInstr->getUnaryOperator()),
            cloneValue(unaryInstr->getSrc()), cloneValue(unaryInstr->getDst()));
    }
    if (auto binaryInstr =
            dynamic_cast<const IR::BinaryInstruction *>(instruction)) {
        return std::make_unique<IR::BinaryInstruction>(
            cloneBinaryOperator(binaryInstr->getBinaryOperator()),
            cloneValue(binaryInstr->getSrc1()),
            cloneValue(binaryInstr->getSrc2()),
            cloneValue(binaryInstr->getDst()));
    }
    if (auto copyInstr =
            dynamic_cast<const IR::CopyInstruction *>(instruction)) {
        return std::make_unique<IR::CopyInstruction>(
            cloneValue(copyInstr->getSrc()), cloneValue(copyInstr->getDst()));
    }
    if (auto jumpInstr =
            dynamic_cast<const IR::JumpInstruction *>(instruction)) {
        return std::make_unique<IR::JumpInstruction>(jumpInstr->getTarget());
    }
    if (auto jumpIfZero =
            dynamic_cast<const IR::JumpIfZeroInstruction *>(instruction)) {
        return std::make_unique<IR::JumpIfZeroInstruction>(
            cloneValue(jumpIfZero->getCondition()), jumpIfZero->getTarget());
    }
    if (auto jumpIfNotZero =
            dynamic_cast<const IR::JumpIfNotZeroInstruction *>(instruction)) {
        return std::make_unique<IR::JumpIfNotZeroInstruction>(
            cloneValue(jumpIfNotZero->getCondition()),
            jumpIfNotZero->getTarget());
    }
    if (auto labelInstr =
            dynamic_cast<const IR::LabelInstruction *>(instruction)) {
        return std::make_unique<IR::LabelInstruction>(labelInstr->getLabel());
    }
    if (auto callInstr =
            dynamic_cast<const IR::FunctionCallInstruction *>(instruction)) {
        auto args = std::make_unique<std::vector<std::unique_ptr<IR::Value>>>();
        for (const auto &arg : callInstr->getArgs()) {
            args->emplace_back(cloneValue(arg.get()));
        }
        return std::make_unique<IR::FunctionCallInstruction>(
            callInstr->getFunctionIdentifier(), std::move(args),
            cloneValue(callInstr->getDst()));
    }
    throw std::logic_error("Unsupported instruction in cloneInstruction");
}

/**
 * Clone a function body represented as a vector of IR instructions.
 *
 * @param functionBody The function body to clone.
 * @return A `std::unique_ptr` to the cloned function body.
 */
std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
cloneFunctionBody(
    const std::vector<std::unique_ptr<IR::Instruction>> &functionBody) {
    auto cloned =
        std::make_unique<std::vector<std::unique_ptr<IR::Instruction>>>();
    cloned->reserve(functionBody.size());
    for (const auto &instruction : functionBody) {
        cloned->emplace_back(cloneInstruction(instruction.get()));
    }
    return cloned;
}
} // namespace

namespace IR {
std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
IROptimizer::irOptimize(
    const std::vector<std::unique_ptr<IR::Instruction>> &functionBody,
    bool foldConstantsPass, bool propagateCopiesPass,
    bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass) {
    auto currentFunctionBody = cloneFunctionBody(functionBody);
    if (foldConstantsPass) {
        currentFunctionBody =
            IR::ConstantFoldingPass::foldConstants(*currentFunctionBody);
    }
    if (eliminateUnreachableCodePass) {
        currentFunctionBody =
            IR::UnreachableCodeEliminationPass::eliminateUnreachableCode(
                *currentFunctionBody);
    }
    if (propagateCopiesPass) {
        currentFunctionBody =
            IR::CopyPropagationPass::propagateCopies(*currentFunctionBody);
    }
    if (eliminateDeadStoresPass) {
        currentFunctionBody = IR::DeadStoreEliminationPass::eliminateDeadStores(
            *currentFunctionBody);
    }
    return currentFunctionBody;
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
ConstantFoldingPass::foldConstants(
    const std::vector<std::unique_ptr<IR::Instruction>> &functionBody) {
    auto folded =
        std::make_unique<std::vector<std::unique_ptr<IR::Instruction>>>();
    folded->reserve(functionBody.size());
    for (const auto &instruction : functionBody) {
        // Handle constant folding for unary instructions.
        if (auto unaryInstr =
                dynamic_cast<const IR::UnaryInstruction *>(instruction.get())) {
            auto srcConst = getConstValue(unaryInstr->getSrc());
            if (srcConst.has_value()) {
                auto foldedConst =
                    foldUnary(unaryInstr->getUnaryOperator(), *srcConst);
                if (foldedConst.has_value()) {
                    folded->emplace_back(std::make_unique<IR::CopyInstruction>(
                        makeConstValue(*foldedConst),
                        cloneValue(unaryInstr->getDst())));
                    continue;
                }
            }
        }
        // Handle constant folding for binary instructions.
        if (auto binaryInstr = dynamic_cast<const IR::BinaryInstruction *>(
                instruction.get())) {
            auto lhsConst = getConstValue(binaryInstr->getSrc1());
            auto rhsConst = getConstValue(binaryInstr->getSrc2());
            if (lhsConst.has_value() && rhsConst.has_value()) {
                auto foldedConst = foldBinary(binaryInstr->getBinaryOperator(),
                                              *lhsConst, *rhsConst);
                if (foldedConst.has_value()) {
                    folded->emplace_back(std::make_unique<IR::CopyInstruction>(
                        makeConstValue(*foldedConst),
                        cloneValue(binaryInstr->getDst())));
                    continue;
                }
            }
        }
        // Handle constant folding for jump instructions.
        if (auto jumpIfZero = dynamic_cast<const IR::JumpIfZeroInstruction *>(
                instruction.get())) {
            auto condConst = getConstValue(jumpIfZero->getCondition());
            if (condConst.has_value()) {
                if (condConst->value == 0) {
                    folded->emplace_back(std::make_unique<IR::JumpInstruction>(
                        jumpIfZero->getTarget()));
                }
                continue;
            }
        }
        // Handle constant folding for jump-if-not-zero instructions.
        if (auto jumpIfNotZero =
                dynamic_cast<const IR::JumpIfNotZeroInstruction *>(
                    instruction.get())) {
            auto condConst = getConstValue(jumpIfNotZero->getCondition());
            if (condConst.has_value()) {
                if (condConst->value != 0) {
                    folded->emplace_back(std::make_unique<IR::JumpInstruction>(
                        jumpIfNotZero->getTarget()));
                }
                continue;
            }
        }
        // If no folding was possible, clone the original instruction.
        folded->emplace_back(cloneInstruction(instruction.get()));
    }
    return folded;
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
CFG::makeControlFlowGraph(
    const std::vector<std::unique_ptr<IR::Instruction>> &functionBody) {
    return cloneFunctionBody(functionBody);
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
CFG::cfgToInstructions(
    const std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return cloneFunctionBody(cfg);
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
UnreachableCodeEliminationPass::eliminateUnreachableCode(
    const std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return cloneFunctionBody(cfg);
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
CopyPropagationPass::propagateCopies(
    const std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return cloneFunctionBody(cfg);
}

std::unique_ptr<std::vector<std::unique_ptr<IR::Instruction>>>
DeadStoreEliminationPass::eliminateDeadStores(
    const std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return cloneFunctionBody(cfg);
}
} // namespace IR
