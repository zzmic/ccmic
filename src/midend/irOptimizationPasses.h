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
/**
 * Class for optimizing IR function bodies using various
 * optimization passes.
 */
class IROptimizer {
  public:
    /**
     * Optimize the given IR function body using the specified optimization
     * passes.
     *
     * @param functionBody The IR function body to optimize.
     * @param foldConstantsPass Boolean indicating whether to apply constant
     * folding.
     * @param propagateCopiesPass Boolean indicating whether to apply copy
     * propagation.
     * @param eliminateUnreachableCodePass Boolean indicating whether to apply
     * unreachable code elimination.
     * @param eliminateDeadStoresPass Boolean indicating whether to apply dead
     * store elimination.
     * @return The optimized IR function body.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    irOptimize(const std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                   &functionBody,
               bool foldConstantsPass, bool propagateCopiesPass,
               bool eliminateUnreachableCodePass, bool eliminateDeadStoresPass);
};

/**
 * Base class for IR optimization passes.
 */
class OptimizationPass {
  public:
    /**
     * Default virtual destructor for the optimization pass class.
     */
    virtual ~OptimizationPass() = default;
};

/**
 * Class for performing constant folding optimization on IR instructions.
 */
class ConstantFoldingPass : public OptimizationPass {
  public:
    /**
     * Perform constant folding on the given IR function body.
     *
     * @param functionBody The IR function body to optimize.
     * @return The optimized IR function body with constants folded.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    foldConstants(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &functionBody);
};

/**
 * Class for constructing control flow graphs (CFGs) from IR instructions.
 *
 * TODO(zzmic): CFGs are temporarily of type
 * `std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>`.
 */
class CFG {
  public:
    /**
     * Convert the given IR function body into a control flow graph (CFG).
     *
     * @param functionBody The IR function body to convert.
     * @return The control flow graph (CFG) representation of the function body.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    makeControlFlowGraph(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &functionBody);

    /**
     * Convert the given control flow graph (CFG) back into a linear sequence
     * of IR instructions.
     *
     * @param cfg The control flow graph (CFG) to convert.
     * @return The linear sequence of IR instructions.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    cfgToInstructions(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &cfg);
};

/**
 * Class for performing unreachable code elimination optimization on IR
 * instructions.
 */
class UnreachableCodeEliminationPass : public OptimizationPass {
  public:
    /**
     * Eliminate unreachable code from the given IR function body.
     *
     * @param cfg The control flow graph (CFG) representation of the function
     * body.
     * @return The optimized IR function body with unreachable code removed.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    eliminateUnreachableCode(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &cfg);
};

/**
 * Class for performing copy propagation optimization on IR instructions.
 */
class CopyPropagationPass : public OptimizationPass {
  public:
    /**
     * Perform copy propagation on the given IR function body.
     *
     * @param cfg The control flow graph (CFG) representation of the function
     * body.
     * @return The optimized IR function body with copies propagated.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    propagateCopies(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &cfg);
};

/**
 * Class for performing dead store elimination optimization on IR
 * instructions.
 */
class DeadStoreEliminationPass : public OptimizationPass {
  public:
    /**
     * Eliminate dead stores from the given IR function body.
     *
     * @param cfg The control flow graph (CFG) representation of the function
     * body.
     * @return The optimized IR function body with dead stores removed.
     */
    [[nodiscard]] static std::shared_ptr<
        std::vector<std::shared_ptr<IR::Instruction>>>
    eliminateDeadStores(
        const std::shared_ptr<std::vector<std::shared_ptr<IR::Instruction>>>
            &cfg);
};
} // namespace IR

#endif // MIDEND_IR_OPTIMIZATION_PASSES_H
