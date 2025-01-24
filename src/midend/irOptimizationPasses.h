#ifndef MIDEND_IR_OPTIMIZATION_PASSES_H
#define MIDEND_IR_OPTIMIZATION_PASSES_H

#include "../frontend/block.h"
#include "../frontend/declaration.h"
#include "../frontend/expression.h"
#include "../frontend/function.h"
#include "../frontend/program.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/statement.h"
#include "../frontend/type.h"
#include "ir.h"

namespace IR {
class OptimizationPass {
  public:
    virtual ~OptimizationPass() = default;
};

class ConstantFoldingPass : public OptimizationPass {
  public:
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    foldConstants(std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
                      functionBody);
};

// TODO(zzmic): CFGs are temporarily of type
// `std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>`.
class CFG {
  public:
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    makeControlFlowGraph(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            functionBody);

    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    cfgToInstructions(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg);
};

class UnreachableCodeEliminationPass : public OptimizationPass {
  public:
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    eliminateUnreachableCode(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg);
};

class CopyPropagationPass : public OptimizationPass {
  public:
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    propagateCopies(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg);
};

class DeadStoreEliminationPass : public OptimizationPass {
  public:
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    eliminateDeadStores(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>> cfg);
};
} // namespace IR

#endif // MIDEND_IR_OPTIMIZATION_PASSES_H
