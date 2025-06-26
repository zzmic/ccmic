#ifndef MIDEND_IR_H
#define MIDEND_IR_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace IR {
class Operator {
  public:
    virtual ~Operator() = default;
};

class UnaryOperator : public Operator {};

class NegateOperator : public UnaryOperator {};

class ComplementOperator : public UnaryOperator {};

class NotOperator : public UnaryOperator {};

/* Note: The logical-and and logical-or operators in the AST are NOT binary
 * operators in the IR. */
class BinaryOperator : public Operator {};

class AddOperator : public BinaryOperator {};

class SubtractOperator : public BinaryOperator {};

class MultiplyOperator : public BinaryOperator {};

class DivideOperator : public BinaryOperator {};

class RemainderOperator : public BinaryOperator {};

class EqualOperator : public BinaryOperator {};

class NotEqualOperator : public BinaryOperator {};

class LessThanOperator : public BinaryOperator {};

class LessThanOrEqualOperator : public BinaryOperator {};

class GreaterThanOperator : public BinaryOperator {};

class GreaterThanOrEqualOperator : public BinaryOperator {};

class Value {
  public:
    virtual ~Value() = default;
};

class ConstantValue : public Value {
  private:
    std::shared_ptr<AST::Constant> astConstant;

  public:
    explicit ConstantValue(const std::shared_ptr<AST::Constant> &astConstant)
        : astConstant(astConstant) {}
    ~ConstantValue() = default;
    [[nodiscard]] std::shared_ptr<AST::Constant> getASTConstant() const {
        return astConstant;
    }
    void setASTConstant(const std::shared_ptr<AST::Constant> &newAstConstant) {
        this->astConstant = newAstConstant;
    }
};

class VariableValue : public Value {
  private:
    std::string identifier;

  public:
    explicit VariableValue(std::string_view identifier)
        : identifier(identifier) {}
    ~VariableValue() = default;
    [[nodiscard]] const std::string &getIdentifier() const {
        return identifier;
    }
    void setIdentifier(std::string_view newIdentifier) {
        this->identifier = newIdentifier;
    }
};

class Instruction {
  public:
    virtual ~Instruction() = default;
};

class ReturnInstruction : public Instruction {
  private:
    std::shared_ptr<Value> returnValue;

  public:
    explicit ReturnInstruction(const std::shared_ptr<Value> &returnValue)
        : returnValue(returnValue) {}
    [[nodiscard]] std::shared_ptr<Value> getReturnValue() const {
        return returnValue;
    }
    void setReturnValue(const std::shared_ptr<Value> &newReturnValue) {
        this->returnValue = newReturnValue;
    }
};

class SignExtendInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    SignExtendInstruction(const std::shared_ptr<Value> &src,
                          const std::shared_ptr<Value> &dst)
        : src(src), dst(dst) {}
    [[nodiscard]] std::shared_ptr<Value> getSrc() const { return src; }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(const std::shared_ptr<Value> &newSrc) { this->src = newSrc; }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class TruncateInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    TruncateInstruction(const std::shared_ptr<Value> &src,
                        const std::shared_ptr<Value> &dst)
        : src(src), dst(dst) {}
    [[nodiscard]] std::shared_ptr<Value> getSrc() const { return src; }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(const std::shared_ptr<Value> &newSrc) { this->src = newSrc; }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<Value> src, dst;

  public:
    UnaryInstruction(const std::shared_ptr<UnaryOperator> &unaryOperator,
                     const std::shared_ptr<Value> &src,
                     const std::shared_ptr<Value> &dst)
        : unaryOperator(unaryOperator), src(src), dst(dst) {}
    [[nodiscard]] std::shared_ptr<UnaryOperator> getUnaryOperator() const {
        return unaryOperator;
    }
    [[nodiscard]] std::shared_ptr<Value> getSrc() const { return src; }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void
    setUnaryOperator(const std::shared_ptr<UnaryOperator> &newUnaryOperator) {
        this->unaryOperator = newUnaryOperator;
    }
    void setSrc(const std::shared_ptr<Value> &newSrc) { this->src = newSrc; }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class BinaryInstruction : public Instruction {
  private:
    std::shared_ptr<BinaryOperator> binaryOperator;
    std::shared_ptr<Value> src1, src2, dst;

  public:
    BinaryInstruction(const std::shared_ptr<BinaryOperator> &binaryOperator,
                      const std::shared_ptr<Value> &src1,
                      const std::shared_ptr<Value> &src2,
                      const std::shared_ptr<Value> &dst)
        : binaryOperator(binaryOperator), src1(src1), src2(src2), dst(dst) {}
    [[nodiscard]] std::shared_ptr<BinaryOperator> getBinaryOperator() const {
        return binaryOperator;
    }
    [[nodiscard]] std::shared_ptr<Value> getSrc1() const { return src1; }
    [[nodiscard]] std::shared_ptr<Value> getSrc2() const { return src2; }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void setBinaryOperator(
        const std::shared_ptr<BinaryOperator> &newBinaryOperator) {
        this->binaryOperator = newBinaryOperator;
    }
    void setSrc1(const std::shared_ptr<Value> &newSrc1) {
        this->src1 = newSrc1;
    }
    void setSrc2(const std::shared_ptr<Value> &newSrc2) {
        this->src2 = newSrc2;
    }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class CopyInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    CopyInstruction(const std::shared_ptr<Value> &src,
                    const std::shared_ptr<Value> &dst)
        : src(src), dst(dst) {}
    [[nodiscard]] std::shared_ptr<Value> getSrc() const { return src; }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(const std::shared_ptr<Value> &newSrc) { this->src = newSrc; }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class JumpInstruction : public Instruction {
  private:
    std::string target;

  public:
    explicit JumpInstruction(std::string_view target) : target(target) {}
    [[nodiscard]] const std::string &getTarget() const { return target; }
    void setTarget(std::string_view newTarget) { this->target = newTarget; }
};

class JumpIfZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfZeroInstruction(const std::shared_ptr<Value> &condition,
                          std::string_view target)
        : condition(condition), target(target) {}
    [[nodiscard]] std::shared_ptr<Value> getCondition() const {
        return condition;
    }
    [[nodiscard]] const std::string &getTarget() const { return target; }
    void setCondition(const std::shared_ptr<Value> &newCondition) {
        this->condition = newCondition;
    }
    void setTarget(std::string_view newTarget) { this->target = newTarget; }
};

class JumpIfNotZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfNotZeroInstruction(const std::shared_ptr<Value> &condition,
                             std::string_view target)
        : condition(condition), target(target) {}
    [[nodiscard]] std::shared_ptr<Value> getCondition() const {
        return condition;
    }
    [[nodiscard]] const std::string &getTarget() const { return target; }
    void setCondition(const std::shared_ptr<Value> &newCondition) {
        this->condition = newCondition;
    }
    void setTarget(std::string_view newTarget) { this->target = newTarget; }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit LabelInstruction(std::string_view label) : label(label) {}
    [[nodiscard]] const std::string &getLabel() const { return label; }
    void setLabel(std::string_view newLabel) { this->label = newLabel; }
};

class FunctionCallInstruction : public Instruction {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::shared_ptr<Value>>> args;
    std::shared_ptr<Value> dst;

  public:
    FunctionCallInstruction(
        std::string_view functionIdentifier,
        const std::shared_ptr<std::vector<std::shared_ptr<Value>>> &args,
        const std::shared_ptr<Value> &dst)
        : functionIdentifier(functionIdentifier), args(args), dst(dst) {}
    [[nodiscard]] const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }
    [[nodiscard]] const std::shared_ptr<std::vector<std::shared_ptr<Value>>> &
    getArgs() const {
        return args;
    }
    [[nodiscard]] std::shared_ptr<Value> getDst() const { return dst; }
    void setFunctionIdentifier(std::string_view newFunctionIdentifier) {
        this->functionIdentifier = newFunctionIdentifier;
    }
    void setArgs(
        const std::shared_ptr<std::vector<std::shared_ptr<Value>>> &newArgs) {
        this->args = newArgs;
    }
    void setDst(const std::shared_ptr<Value> &newDst) { this->dst = newDst; }
};

class TopLevel {
  public:
    virtual ~TopLevel() = default;
};

class FunctionDefinition : public TopLevel {
  private:
    std::string functionIdentifier;
    bool global;
    std::shared_ptr<std::vector<std::string>> parameters;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;

  public:
    FunctionDefinition(
        std::string_view functionIdentifier, bool global,
        const std::shared_ptr<std::vector<std::string>> &parameters,
        const std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
            &functionBody)
        : functionIdentifier(functionIdentifier), global(global),
          parameters(parameters), functionBody(functionBody) {}
    [[nodiscard]] const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }
    [[nodiscard]] bool isGlobal() const { return global; }
    [[nodiscard]] const std::shared_ptr<std::vector<std::string>> &
    getParameterIdentifiers() const {
        return parameters;
    }
    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<Instruction>>> &
    getFunctionBody() const {
        return functionBody;
    }
    void setFunctionBody(
        const std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
            &newFunctionBody) {
        this->functionBody = newFunctionBody;
    }
};

class StaticVariable : public TopLevel {
  private:
    std::string identifier;
    bool global;
    std::shared_ptr<AST::Type> type;
    std::shared_ptr<AST::StaticInit> staticInit;

  public:
    StaticVariable(std::string_view identifier, bool global,
                   const std::shared_ptr<AST::Type> &type,
                   const std::shared_ptr<AST::StaticInit> &staticInit)
        : identifier(identifier), global(global), type(type),
          staticInit(staticInit) {}
    [[nodiscard]] const std::string &getIdentifier() const {
        return identifier;
    }
    [[nodiscard]] bool isGlobal() const { return global; }
    [[nodiscard]] std::shared_ptr<AST::Type> getType() const { return type; }
    [[nodiscard]] std::shared_ptr<AST::StaticInit> getStaticInit() const {
        return staticInit;
    }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels;

  public:
    explicit Program(
        const std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>>
            &topLevels)
        : topLevels(topLevels) {}
    [[nodiscard]] const std::shared_ptr<
        std::vector<std::shared_ptr<TopLevel>>> &
    getTopLevels() const {
        return topLevels;
    }
};
} // namespace IR

#endif // MIDEND_IR_H
