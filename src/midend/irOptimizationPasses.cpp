#include "irOptimizationPasses.h"

namespace IR {
namespace {
std::unique_ptr<AST::Constant> cloneASTConstant(const AST::Constant *constant) {
    if (auto intConst = dynamic_cast<const AST::ConstantInt *>(constant)) {
        return std::make_unique<AST::ConstantInt>(intConst->getValue());
    }
    if (auto longConst = dynamic_cast<const AST::ConstantLong *>(constant)) {
        return std::make_unique<AST::ConstantLong>(longConst->getValue());
    }
    throw std::logic_error("Unsupported AST constant in cloneASTConstant");
}

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
    return cloneFunctionBody(functionBody);
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
