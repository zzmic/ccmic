#include "irOptimizationPasses.h"
#include <limits>

namespace IR {
// Helper function to perform optimization passes on the IR function definition.
std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
IROptimizer::irOptimize(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &functionBody,
    bool foldConstantsPass, bool propagateCopiesPass,
    bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass) {
    if (functionBody->empty()) {
        return functionBody;
    }
    auto currentFunctionBody = functionBody;
    while (true) {
        auto postConstantFoldingFunctionBody = currentFunctionBody;
        if (foldConstantsPass) {
            postConstantFoldingFunctionBody =
                IR::ConstantFoldingPass::foldConstants(currentFunctionBody);
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
        if (optimizedFunctionBody == currentFunctionBody ||
            optimizedFunctionBody->empty()) {
            return optimizedFunctionBody;
        }
        currentFunctionBody = optimizedFunctionBody;
    }
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
ConstantFoldingPass::foldConstants(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &functionBody) {
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
                auto unaryOperator = unaryInstruction->getUnaryOperator();
                auto astConstant = constantValue->getASTConstant();
                if (std::dynamic_pointer_cast<IR::NegateOperator>(
                        unaryOperator)) {
                    if (auto constantInt =
                            std::dynamic_pointer_cast<AST::ConstantInt>(
                                astConstant)) {
                        astConstant = std::make_shared<AST::ConstantInt>(
                            -constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 std::dynamic_pointer_cast<AST::ConstantLong>(
                                     astConstant)) {
                        astConstant = std::make_shared<AST::ConstantLong>(
                            -constantLong->getValue());
                    }
                }
                else if (std::dynamic_pointer_cast<IR::ComplementOperator>(
                             unaryOperator)) {
                    if (auto constantInt =
                            std::dynamic_pointer_cast<AST::ConstantInt>(
                                astConstant)) {
                        astConstant = std::make_shared<AST::ConstantInt>(
                            ~constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 std::dynamic_pointer_cast<AST::ConstantLong>(
                                     astConstant)) {
                        astConstant = std::make_shared<AST::ConstantLong>(
                            ~constantLong->getValue());
                    }
                }
                else if (std::dynamic_pointer_cast<IR::NotOperator>(
                             unaryOperator)) {
                    if (auto constantInt =
                            std::dynamic_pointer_cast<AST::ConstantInt>(
                                astConstant)) {
                        astConstant = std::make_shared<AST::ConstantInt>(
                            !constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 std::dynamic_pointer_cast<AST::ConstantLong>(
                                     astConstant)) {
                        astConstant = std::make_shared<AST::ConstantLong>(
                            !constantLong->getValue());
                    }
                }
                else {
                    throw std::logic_error("Unsupported unary operator");
                }
                auto copyInstruction = std::make_shared<IR::CopyInstruction>(
                    std::make_shared<IR::ConstantValue>(astConstant),
                    unaryInstruction->getDst());
                *it = std::move(copyInstruction);
            }
            ++it;
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
                        auto astConstant1 = constantValue1->getASTConstant();
                        auto astConstant2 = constantValue2->getASTConstant();

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    astConstant1)) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(astConstant1)) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    astConstant2)) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(astConstant2)) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        if ((value1 > 0 &&
                             value2 >
                                 std::numeric_limits<int>::max() - value1) ||
                            (value1 < 0 &&
                             value2 <
                                 std::numeric_limits<int>::min() - value1)) {
                            ++it;
                            continue;
                        }

                        constantResult = value1 + value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }

                        if ((value1 > 0 &&
                             value2 <
                                 std::numeric_limits<int>::min() + value1) ||
                            (value1 < 0 &&
                             value2 >
                                 std::numeric_limits<int>::max() - value1)) {
                            ++it;
                            continue;
                        }
                        constantResult = value1 - value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 * value2;
                        if (value1 != 0 && constantResult / value1 != value2) {
                            ++it;
                            continue;
                        }
                    }
                    else if (std::dynamic_pointer_cast<IR::DivideOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        if (value2 == 0) {
                            ++it;
                            continue;
                        }
                        constantResult = value1 / value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        if (value2 == 0) {
                            ++it;
                            continue;
                        }
                        constantResult = value1 % value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::EqualOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 == value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::NotEqualOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 != value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::LessThanOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 < value2;
                    }
                    else if (std::dynamic_pointer_cast<
                                 IR::LessThanOrEqualOperator>(binaryOperator)) {
                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 <= value2;
                    }
                    else if (std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 > value2;
                    }
                    else if (std::dynamic_pointer_cast<
                                 IR::GreaterThanOrEqualOperator>(
                                 binaryOperator)) {

                        int value1 = 0, value2 = 0;
                        if (auto constInt1 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue1->getASTConstant())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue1->getASTConstant())) {
                            value1 = static_cast<int>(constLong1->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 =
                                std::dynamic_pointer_cast<AST::ConstantInt>(
                                    constantValue2->getASTConstant())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 = std::dynamic_pointer_cast<
                                     AST::ConstantLong>(
                                     constantValue2->getASTConstant())) {
                            value2 = static_cast<int>(constLong2->getValue());
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 >= value2;
                    }
                    else {
                        throw std::logic_error("Unsupported binary operator");
                    }

                    auto copyInstruction =
                        std::make_shared<IR::CopyInstruction>(
                            std::make_shared<IR::ConstantValue>(
                                std::make_shared<AST::ConstantInt>(
                                    constantResult)),
                            binaryInstruction->getDst());
                    *it = std::move(copyInstruction);
                }
            }
            ++it;
        }
        // Handle `JumpIfZero` instructions.
        else if (auto jumpIfZeroInstruction =
                     std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                         instruction)) {
            if (auto constantValue =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        jumpIfZeroInstruction->getCondition())) {
                // Get the actual value from AST constant.
                int conditionValue = 0;
                if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                        constantValue->getASTConstant())) {
                    conditionValue = constInt->getValue();
                }
                else if (auto constLong =
                             std::dynamic_pointer_cast<AST::ConstantLong>(
                                 constantValue->getASTConstant())) {
                    conditionValue = static_cast<int>(constLong->getValue());
                }
                else {
                    throw std::logic_error("Unsupported constant type");
                }

                if (conditionValue == 0) {
                    *it = std::make_shared<IR::JumpInstruction>(
                        jumpIfZeroInstruction->getTarget());
                    ++it;
                }
                else {
                    it = postConstantFoldingFunctionBody->erase(it);
                }
            }
            else {
                ++it;
            }
        }
        // Handle `JumpIfNotZero` instructions.
        else if (auto jumpIfNotZeroInstruction =
                     std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                         instruction)) {
            if (auto constantValue =
                    std::dynamic_pointer_cast<IR::ConstantValue>(
                        jumpIfNotZeroInstruction->getCondition())) {
                // Get the actual value from AST constant.
                int conditionValue = 0;
                if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                        constantValue->getASTConstant())) {
                    conditionValue = constInt->getValue();
                }
                else if (auto constLong =
                             std::dynamic_pointer_cast<AST::ConstantLong>(
                                 constantValue->getASTConstant())) {
                    conditionValue = static_cast<int>(constLong->getValue());
                }
                else {
                    throw std::logic_error("Unsupported constant type");
                }

                if (conditionValue != 0) {
                    *it = std::make_shared<IR::JumpInstruction>(
                        jumpIfNotZeroInstruction->getTarget());
                    ++it;
                }
                else {
                    it = postConstantFoldingFunctionBody->erase(it);
                }
            }
            else {
                ++it;
            }
        }
        else {
            ++it;
        }
    }
    return postConstantFoldingFunctionBody;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CFG::makeControlFlowGraph(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        &functionBody) {
    return functionBody;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CFG::cfgToInstructions(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> &cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
UnreachableCodeEliminationPass::eliminateUnreachableCode(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> &cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CopyPropagationPass::propagateCopies(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> &cfg) {
    return cfg;
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
DeadStoreEliminationPass::eliminateDeadStores(
    const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> &cfg) {
    return cfg;
}
} // namespace IR
