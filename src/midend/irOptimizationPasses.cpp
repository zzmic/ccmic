#include "irOptimizationPasses.h"
#include <limits>

namespace IR {
// Helper function to perform optimization passes on the IR function definition.
std::vector<std::unique_ptr<IR::Instruction>> IROptimizer::irOptimize(
    std::vector<std::unique_ptr<IR::Instruction>> &functionBody,
    bool foldConstantsPass, bool propagateCopiesPass,
    bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass) {
    if (functionBody.empty()) {
        return std::move(functionBody);
    }
    auto currentFunctionBody = std::move(functionBody);
    // Limit iterations to prevent infinite loops
    const int maxIterations = 10;
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        auto postConstantFoldingFunctionBody = std::move(currentFunctionBody);
        if (foldConstantsPass) {
            postConstantFoldingFunctionBody =
                IR::ConstantFoldingPass::foldConstants(
                    postConstantFoldingFunctionBody);
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
        if (optimizedFunctionBody.empty()) {
            return optimizedFunctionBody;
        }
        currentFunctionBody = std::move(optimizedFunctionBody);
    }
    return currentFunctionBody;
}

std::vector<std::unique_ptr<IR::Instruction>>
ConstantFoldingPass::foldConstants(
    std::vector<std::unique_ptr<IR::Instruction>> &functionBody) {
    auto postConstantFoldingFunctionBody = std::move(functionBody);
    for (auto it = postConstantFoldingFunctionBody.begin();
         it != postConstantFoldingFunctionBody.end();) {
        auto instruction = it->get();
        // Handle unary instructions with a constant source operand.
        if (auto unaryInstruction =
                dynamic_cast<IR::UnaryInstruction *>(instruction)) {
            if (auto constantValue = dynamic_cast<IR::ConstantValue *>(
                    unaryInstruction->getSrc().get())) {
                auto unaryOperator =
                    std::move(unaryInstruction->getUnaryOperator());
                auto astConstant = std::move(constantValue->getASTConstant());
                if (dynamic_cast<IR::NegateOperator *>(unaryOperator.get())) {
                    if (auto constantInt = dynamic_cast<AST::ConstantInt *>(
                            astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantInt>(
                            -constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 dynamic_cast<AST::ConstantLong *>(
                                     astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantLong>(
                            -constantLong->getValue());
                    }
                }
                else if (dynamic_cast<IR::ComplementOperator *>(
                             unaryOperator.get())) {
                    if (auto constantInt = dynamic_cast<AST::ConstantInt *>(
                            astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantInt>(
                            ~constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 dynamic_cast<AST::ConstantLong *>(
                                     astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantLong>(
                            ~constantLong->getValue());
                    }
                }
                else if (dynamic_cast<IR::NotOperator *>(unaryOperator.get())) {
                    if (auto constantInt = dynamic_cast<AST::ConstantInt *>(
                            astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantInt>(
                            !constantInt->getValue());
                    }
                    else if (auto constantLong =
                                 dynamic_cast<AST::ConstantLong *>(
                                     astConstant.get())) {
                        astConstant = std::make_unique<AST::ConstantLong>(
                            !constantLong->getValue());
                    }
                }
                else {
                    throw std::logic_error("Unsupported unary operator");
                }
                auto copyInstruction = std::make_unique<IR::CopyInstruction>(
                    std::make_unique<IR::ConstantValue>(std::move(astConstant)),
                    std::move(unaryInstruction->getDst()));
                *it = std::move(copyInstruction);
            }
            ++it;
        }
        // Handle binary instructions with two constant source operands.
        else if (auto binaryInstruction =
                     dynamic_cast<IR::BinaryInstruction *>(instruction)) {
            if (auto constantValue1 = dynamic_cast<IR::ConstantValue *>(
                    binaryInstruction->getSrc1().get())) {
                if (auto constantValue2 = dynamic_cast<IR::ConstantValue *>(
                        binaryInstruction->getSrc2().get())) {
                    auto binaryOperator =
                        std::move(binaryInstruction->getBinaryOperator());
                    long constantResult = 0;
                    if (dynamic_cast<IR::AddOperator *>(binaryOperator.get())) {
                        auto astConstant1 =
                            std::move(constantValue1->getASTConstant());
                        auto astConstant2 =
                            std::move(constantValue2->getASTConstant());

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                astConstant1.get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         astConstant1.get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                astConstant2.get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         astConstant2.get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        // Check for overflow in 64-bit arithmetic.
                        if ((value1 > 0 &&
                             value2 >
                                 std::numeric_limits<long>::max() - value1) ||
                            (value1 < 0 &&
                             value2 <
                                 std::numeric_limits<long>::min() - value1)) {
                            ++it;
                            continue;
                        }

                        constantResult = value1 + value2;
                    }
                    else if (dynamic_cast<IR::SubtractOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }

                        // Check for overflow in 64-bit arithmetic.
                        if ((value1 > 0 &&
                             value2 <
                                 std::numeric_limits<long>::min() + value1) ||
                            (value1 < 0 &&
                             value2 >
                                 std::numeric_limits<long>::max() - value1)) {
                            ++it;
                            continue;
                        }
                        constantResult = value1 - value2;
                    }
                    else if (dynamic_cast<IR::MultiplyOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
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
                    else if (dynamic_cast<IR::DivideOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
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
                    else if (dynamic_cast<IR::RemainderOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
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
                    else if (dynamic_cast<IR::EqualOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 == value2;
                    }
                    else if (dynamic_cast<IR::NotEqualOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 != value2;
                    }
                    else if (dynamic_cast<IR::LessThanOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 < value2;
                    }
                    else if (dynamic_cast<IR::LessThanOrEqualOperator *>(
                                 binaryOperator.get())) {
                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 <= value2;
                    }
                    else if (dynamic_cast<IR::GreaterThanOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 > value2;
                    }
                    else if (dynamic_cast<IR::GreaterThanOrEqualOperator *>(
                                 binaryOperator.get())) {

                        long value1 = 0, value2 = 0;
                        if (auto constInt1 = dynamic_cast<AST::ConstantInt *>(
                                constantValue1->getASTConstant().get())) {
                            value1 = constInt1->getValue();
                        }
                        else if (auto constLong1 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue1->getASTConstant()
                                             .get())) {
                            value1 = constLong1->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }
                        if (auto constInt2 = dynamic_cast<AST::ConstantInt *>(
                                constantValue2->getASTConstant().get())) {
                            value2 = constInt2->getValue();
                        }
                        else if (auto constLong2 =
                                     dynamic_cast<AST::ConstantLong *>(
                                         constantValue2->getASTConstant()
                                             .get())) {
                            value2 = constLong2->getValue();
                        }
                        else {
                            throw std::logic_error("Unsupported constant type");
                        }

                        constantResult = value1 >= value2;
                    }
                    else {
                        throw std::logic_error("Unsupported binary operator");
                    }

                    // Determine the result type based on input types.
                    bool isLongResult = false;
                    if (dynamic_cast<AST::ConstantLong *>(
                            constantValue1->getASTConstant().get())) {
                        isLongResult = true;
                    }
                    else if (dynamic_cast<AST::ConstantLong *>(
                                 constantValue2->getASTConstant().get())) {
                        isLongResult = true;
                    }

                    std::unique_ptr<IR::ConstantValue> resultConstant;
                    if (isLongResult) {
                        resultConstant = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantLong>(
                                constantResult));
                    }
                    else {
                        resultConstant = std::make_unique<IR::ConstantValue>(
                            std::make_unique<AST::ConstantInt>(
                                static_cast<int>(constantResult)));
                    }

                    auto copyInstruction =
                        std::make_unique<IR::CopyInstruction>(
                            std::move(resultConstant),
                            std::move(binaryInstruction->getDst()));
                    *it = std::move(copyInstruction);
                }
            }
            ++it;
        }
        // Handle `JumpIfZero` instructions.
        else if (auto jumpIfZeroInstruction =
                     dynamic_cast<IR::JumpIfZeroInstruction *>(instruction)) {
            if (auto constantValue = dynamic_cast<IR::ConstantValue *>(
                    jumpIfZeroInstruction->getCondition().get())) {
                // Get the actual value from the AST constant.
                long conditionValue = 0;
                if (auto constInt = dynamic_cast<AST::ConstantInt *>(
                        constantValue->getASTConstant().get())) {
                    conditionValue = constInt->getValue();
                }
                else if (auto constLong = dynamic_cast<AST::ConstantLong *>(
                             constantValue->getASTConstant().get())) {
                    conditionValue = constLong->getValue();
                }
                else {
                    throw std::logic_error("Unsupported constant type");
                }

                if (conditionValue == 0) {
                    *it = std::make_unique<IR::JumpInstruction>(
                        jumpIfZeroInstruction->getTarget());
                    ++it;
                }
                else {
                    it = postConstantFoldingFunctionBody.erase(it);
                }
            }
            else {
                ++it;
            }
        }
        // Handle `JumpIfNotZero` instructions.
        else if (auto jumpIfNotZeroInstruction =
                     dynamic_cast<IR::JumpIfNotZeroInstruction *>(
                         instruction)) {
            if (auto constantValue = dynamic_cast<IR::ConstantValue *>(
                    jumpIfNotZeroInstruction->getCondition().get())) {
                // Get the actual value from the AST constant.
                long conditionValue = 0;
                if (auto constInt = dynamic_cast<AST::ConstantInt *>(
                        constantValue->getASTConstant().get())) {
                    conditionValue = constInt->getValue();
                }
                else if (auto constLong = dynamic_cast<AST::ConstantLong *>(
                             constantValue->getASTConstant().get())) {
                    conditionValue = constLong->getValue();
                }
                else {
                    throw std::logic_error("Unsupported constant type");
                }

                if (conditionValue != 0) {
                    *it = std::make_unique<IR::JumpInstruction>(
                        jumpIfNotZeroInstruction->getTarget());
                    ++it;
                }
                else {
                    it = postConstantFoldingFunctionBody.erase(it);
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

std::vector<std::unique_ptr<IR::Instruction>> CFG::makeControlFlowGraph(
    std::vector<std::unique_ptr<IR::Instruction>> &functionBody) {
    return std::move(functionBody);
}

std::vector<std::unique_ptr<IR::Instruction>>
CFG::cfgToInstructions(std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return std::move(cfg);
}

std::vector<std::unique_ptr<IR::Instruction>>
UnreachableCodeEliminationPass::eliminateUnreachableCode(
    std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return std::move(cfg);
}

std::vector<std::unique_ptr<IR::Instruction>>
CopyPropagationPass::propagateCopies(
    std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return std::move(cfg);
}

std::vector<std::unique_ptr<IR::Instruction>>
DeadStoreEliminationPass::eliminateDeadStores(
    std::vector<std::unique_ptr<IR::Instruction>> &cfg) {
    return std::move(cfg);
}
} // namespace IR
