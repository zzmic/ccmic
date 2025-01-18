#include "irOptimizationPasses.h"

namespace IR {
std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
ConstantFoldingPass::foldConstants(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        functionBody) {
    // TODO(zzmic): This is a stub implementation.
    return functionBody;
}

std::shared_ptr<CFG> CFG::makeControlFlowGraph(
    std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
        functionBody) {
    // TODO(zzmic): This is a stub implementation.
    return std::make_shared<CFG>();
}

std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
CFG::cfgToInstructions(std::shared_ptr<CFG> cfg) {
    // TODO(zzmic): This is a stub implementation.
    return std::make_shared<std::vector<std::shared_ptr<IR::Instruction>>>();
}

std::shared_ptr<CFG> UnreachableCodeEliminationPass::eliminateUnreachableCode(
    std::shared_ptr<CFG> cfg) {
    // TODO(zzmic): This is a stub implementation.
    return cfg;
}

std::shared_ptr<CFG>
CopyPropagationPass::propagateCopies(std::shared_ptr<CFG> cfg) {
    // TODO(zzmic): This is a stub implementation.
    return cfg;
}

std::shared_ptr<CFG>
DeadStoreEliminationPass::eliminateDeadStores(std::shared_ptr<CFG> cfg) {
    // TODO(zzmic): This is a stub implementation.
    return cfg;
}
} // namespace IR
