#ifndef MIDEND_IR_OPTIMIZATION_PASSES_H
#define MIDEND_IR_OPTIMIZATION_PASSES_H

#include "../frontend/block.h"
#include "../frontend/declaration.h"
#include "../frontend/expression.h"
#include "../frontend/function.h"
#include "../frontend/program.h"
#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/statement.h"
#include "../frontend/types.h"
#include "ir.h"
#include <optional>
#include <unordered_map>

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

class CFG {
  public:
    static std::shared_ptr<CFG> makeControlFlowGraph(
        std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            functionBody);
    static std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
    cfgToInstructions(std::shared_ptr<CFG> cfg);
};

class UnreachableCodeEliminationPass : public OptimizationPass {
  public:
    static std::shared_ptr<CFG>
    eliminateUnreachableCode(std::shared_ptr<CFG> cfg);
};

class CopyPropagationPass : public OptimizationPass {
  public:
    static std::shared_ptr<CFG> propagateCopies(std::shared_ptr<CFG> cfg);
};

class DeadStoreEliminationPass : public OptimizationPass {
  public:
    static std::shared_ptr<CFG> eliminateDeadStores(std::shared_ptr<CFG> cfg);
};
} // namespace IR

#endif // MIDEND_IR_OPTIMIZATION_PASSES_H
