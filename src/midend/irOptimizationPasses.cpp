#include "irOptimizationPasses.h"
#include <limits>

namespace IR {
// Helper function to perform optimization passes on the IR function definition.
std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
IROptimizer::irOptimize(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> functionBody,
    bool foldConstantsPass, bool propagateCopiesPass,
    bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass) {
    // Preemptively return the (original) function body if it is empty.
    if (functionBody->empty()) {
        return functionBody;
    }
    while (true) {
        auto postConstantFoldingFunctionBody = functionBody;
        if (foldConstantsPass) {
            postConstantFoldingFunctionBody =
                IR::ConstantFoldingPass::foldConstants(functionBody);
        }
        auto cfg =
            IR::CFG::makeControlFlowGraph(postConstantFoldingFunctionBody);
        if (eliminateUnreachableCodePass) {
            cfg = IR::UnreachableCodeEliminationPass::eliminateUnreachableCode(
                cfg);
        }
        if (propagateCopiesPass) {
            cfg = IR::CopyPropagationPass::propagateCopies(cfg);
        }
        if (eliminateDeadStoresPass) {
            cfg = IR::DeadStoreEliminationPass::eliminateDeadStores(cfg);
        }
        auto optimizedFunctionBody = IR::CFG::cfgToInstructions(cfg);
        if (optimizedFunctionBody == functionBody ||
            optimizedFunctionBody->empty()) {
            return optimizedFunctionBody;
        }
        functionBody = optimizedFunctionBody;
    }
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
ConstantFoldingPass::foldConstants(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        functionBody) {
    auto postConstantFoldingFunctionBody = functionBody;
    for (auto it = postConstantFoldingFunctionBody->begin();
         it != postConstantFoldingFunctionBody->end();) {
        auto instruction = *it;
        // Handle unary instructions with a constant source operand.
        if (auto unaryInstruction =
                std::dynamic_pointer_cast<IR::UnaryInstruction>(instruction)) {
            if (auto constantValue =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        unaryInstruction->getSrc())) {
                auto constantResult = constantValue->getValue();
                auto unaryOperator = unaryInstruction->getUnaryOperator();
                if (std::dynamic_pointer_cast<IR::NegateOperator>(
                        unaryOperator)) {
                    constantResult = -constantResult;
                }
                else if (std::dynamic_pointer_cast<IR::ComplementOperator>(
                             unaryOperator)) {
                    constantResult = ~constantResult;
                }
                else if (std::dynamic_pointer_cast<IR::NotOperator>(
                             unaryOperator)) {
                    constantResult = !constantResult;
                }
                auto copyInstruction = std::make_shared<IR::CopyInstruction>(
                    std::make_shared<IR::ConstantValue>(constantResult),
                    unaryInstruction->getDst());
                *it = std::move(copyInstruction);
            }
            it++;
        }
        // Handle binary instructions with two constant source operands.
        else if (auto binaryInstruction =
                     std::dynamic_pointer_cast<IR::BinaryInstruction>(
                         instruction)) {
            if (auto constantValue1 =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        binaryInstruction->getSrc1())) {
                if (auto constantValue2 =
                        std::dynamic_pointer_cast<IR::ConstantValue>(
                            binaryInstruction->getSrc2())) {
                    auto binaryOperator =
                        binaryInstruction->getBinaryOperator();
                    int constantResult = 0;
                    if (std::dynamic_pointer_cast<IR::AddOperator>(
                            binaryOperator)) {
                        if ((constantValue1->getValue() > 0 &&
                             constantValue2->getValue() >
                                 std::numeric_limits<int>::max() -
                                     constantValue1->getValue()) ||
                            (constantValue1->getValue() < 0 &&
                             constantValue2->getValue() <
                                 std::numeric_limits<int>::min() -
                                     constantValue1->getValue())) {
                            it++;
                            continue;
                        }
                        constantResult = constantValue1->getValue() +
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                                 binaryOperator)) {
                        if ((constantValue1->getValue() > 0 &&
                             constantValue2->getValue() <
                                 std::numeric_limits<int>::min() +
                                     constantValue1->getValue()) ||
                            (constantValue1->getValue() < 0 &&
                             constantValue2->getValue() >
                                 std::numeric_limits<int>::max() -
                                     constantValue1->getValue())) {
                            it++;
                            continue;
                        }
                        constantResult = constantValue1->getValue() -
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() *
                                         constantValue2->getValue();
                        if (constantValue1->getValue() != 0 &&
                            constantResult / constantValue1->getValue() !=
                                constantValue2->getValue()) {
                            it++;
                            continue;
                        }
                    }
                    else if (std::dynamic_pointer_cast<IR::DivideOperator>(
                                 binaryOperator)) {
                        if (constantValue2->getValue() == 0) {
                            it++;
                            continue;
                        }
                        constantResult = constantValue1->getValue() /
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                                 binaryOperator)) {
                        if (constantValue2->getValue() == 0) {
                            it++;
                            continue;
                        }
                        constantResult = constantValue1->getValue() %
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::EqualOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() ==
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::NotEqualOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() !=
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::LessThanOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() <
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<
                                 IR::LessThanOrEqualOperator>(binaryOperator)) {
                        constantResult = constantValue1->getValue() <=
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() >
                                         constantValue2->getValue();
                    }
                    else if (std::dynamic_pointer_cast<
                                 IR::GreaterThanOrEqualOperator>(
                                 binaryOperator)) {
                        constantResult = constantValue1->getValue() >=
                                         constantValue2->getValue();
                    }
                    auto copyInstruction =
                        std::make_shared<IR::CopyInstruction>(
                            std::make_shared<IR::ConstantValue>(constantResult),
                            binaryInstruction->getDst());
                    *it = std::move(copyInstruction);
                }
            }
            it++;
        }
        // Handle `JumpIfZero` instructions.
        else if (auto jumpIfZeroInstruction =
                     std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                         instruction)) {
            if (auto constantValue =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        jumpIfZeroInstruction->getCondition())) {
                if (constantValue->getValue() == 0) {
                    *it = std::make_shared<IR::JumpInstruction>(
                        jumpIfZeroInstruction->getTarget());
                    it++;
                }
                else {
                    it = postConstantFoldingFunctionBody->erase(it);
                }
            }
            else {
                it++;
            }
        }
        // Handle `JumpIfNotZero` instructions.
        else if (auto jumpIfNotZeroInstruction =
                     std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                         instruction)) {
            if (auto constantValue =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        jumpIfNotZeroInstruction->getCondition())) {
                if (constantValue->getValue() != 0) {
                    *it = std::make_shared<IR::JumpInstruction>(
                        jumpIfNotZeroInstruction->getTarget());
                    it++;
                }
                else {
                    it = postConstantFoldingFunctionBody->erase(it);
                }
            }
            else {
                it++;
            }
        }
        else {
            it++;
        }
    }
    return postConstantFoldingFunctionBody;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CFG::makeControlFlowGraph(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        functionBody) {
    return functionBody;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CFG::cfgToInstructions(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
UnreachableCodeEliminationPass::eliminateUnreachableCode(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CopyPropagationPass::propagateCopies(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
DeadStoreEliminationPass::eliminateDeadStores(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg) {
    return cfg;
}
} // namespace IR
