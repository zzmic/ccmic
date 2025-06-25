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
    ConstantValue(std::shared_ptr<AST::Constant> astConstant)
        : astConstant(astConstant) {}
    std::shared_ptr<AST::Constant> getASTConstant() const {
        return astConstant;
    }
    void setASTConstant(std::shared_ptr<AST::Constant> astConstant) {
        this->astConstant = astConstant;
    }
};

class VariableValue : public Value {
  private:
    std::string identifier;

  public:
    VariableValue(std::string_view identifier) : identifier(identifier) {}
    const std::string &getIdentifier() const { return identifier; }
    void setIdentifier(std::string_view identifier) {
        this->identifier = identifier;
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
    ReturnInstruction(std::shared_ptr<Value> returnValue)
        : returnValue(returnValue) {}
    std::shared_ptr<Value> getReturnValue() const { return returnValue; }
    void setReturnValue(std::shared_ptr<Value> returnValue) {
        this->returnValue = returnValue;
    }
};

class SignExtendInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    SignExtendInstruction(std::shared_ptr<Value> src,
                          std::shared_ptr<Value> dst)
        : src(src), dst(dst) {}
    std::shared_ptr<Value> getSrc() const { return src; }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(std::shared_ptr<Value> src) { this->src = src; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class TruncateInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    TruncateInstruction(std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : src(src), dst(dst) {}
    std::shared_ptr<Value> getSrc() const { return src; }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(std::shared_ptr<Value> src) { this->src = src; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<Value> src, dst;

  public:
    UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                     std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : unaryOperator(unaryOperator), src(src), dst(dst) {}
    std::shared_ptr<UnaryOperator> getUnaryOperator() const {
        return unaryOperator;
    }
    std::shared_ptr<Value> getSrc() const { return src; }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setUnaryOperator(std::shared_ptr<UnaryOperator> unaryOperator) {
        this->unaryOperator = unaryOperator;
    }
    void setSrc(std::shared_ptr<Value> src) { this->src = src; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class BinaryInstruction : public Instruction {
  private:
    std::shared_ptr<BinaryOperator> binaryOperator;
    std::shared_ptr<Value> src1, src2, dst;

  public:
    BinaryInstruction(std::shared_ptr<BinaryOperator> binaryOperator,
                      std::shared_ptr<Value> src1, std::shared_ptr<Value> src2,
                      std::shared_ptr<Value> dst)
        : binaryOperator(binaryOperator), src1(src1), src2(src2), dst(dst) {}
    std::shared_ptr<BinaryOperator> getBinaryOperator() const {
        return binaryOperator;
    }
    std::shared_ptr<Value> getSrc1() const { return src1; }
    std::shared_ptr<Value> getSrc2() const { return src2; }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setBinaryOperator(std::shared_ptr<BinaryOperator> binaryOperator) {
        this->binaryOperator = binaryOperator;
    }
    void setSrc1(std::shared_ptr<Value> src1) { this->src1 = src1; }
    void setSrc2(std::shared_ptr<Value> src2) { this->src2 = src2; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class CopyInstruction : public Instruction {
  private:
    std::shared_ptr<Value> src, dst;

  public:
    CopyInstruction(std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : src(src), dst(dst) {}
    std::shared_ptr<Value> getSrc() const { return src; }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setSrc(std::shared_ptr<Value> src) { this->src = src; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class JumpInstruction : public Instruction {
  private:
    std::string target;

  public:
    JumpInstruction(std::string_view target) : target(target) {}
    const std::string &getTarget() const { return target; }
    void setTarget(std::string_view target) { this->target = target; }
};

class JumpIfZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfZeroInstruction(std::shared_ptr<Value> condition,
                          std::string_view target)
        : condition(condition), target(target) {}
    std::shared_ptr<Value> getCondition() const { return condition; }
    const std::string &getTarget() const { return target; }
    void setCondition(std::shared_ptr<Value> condition) {
        this->condition = condition;
    }
    void setTarget(std::string_view target) { this->target = target; }
};

class JumpIfNotZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfNotZeroInstruction(std::shared_ptr<Value> condition,
                             std::string_view target)
        : condition(condition), target(target) {}
    std::shared_ptr<Value> getCondition() const { return condition; }
    const std::string &getTarget() const { return target; }
    void setCondition(std::shared_ptr<Value> condition) {
        this->condition = condition;
    }
    void setTarget(std::string_view target) { this->target = target; }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    LabelInstruction(std::string_view label) : label(label) {}
    const std::string &getLabel() const { return label; }
    void setLabel(std::string_view label) { this->label = label; }
};

class FunctionCallInstruction : public Instruction {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::shared_ptr<Value>>> args;
    std::shared_ptr<Value> dst;

  public:
    FunctionCallInstruction(
        std::string_view functionIdentifier,
        std::shared_ptr<std::vector<std::shared_ptr<Value>>> args,
        std::shared_ptr<Value> dst)
        : functionIdentifier(functionIdentifier), args(args), dst(dst) {}
    const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }
    const std::shared_ptr<std::vector<std::shared_ptr<Value>>> &
    getArgs() const {
        return args;
    }
    std::shared_ptr<Value> getDst() const { return dst; }
    void setFunctionIdentifier(std::string_view functionIdentifier) {
        this->functionIdentifier = functionIdentifier;
    }
    void setArgs(std::shared_ptr<std::vector<std::shared_ptr<Value>>> args) {
        this->args = args;
    }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
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
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody)
        : functionIdentifier(functionIdentifier), global(global),
          parameters(parameters), functionBody(functionBody) {}
    const std::string &getFunctionIdentifier() const {
        return functionIdentifier;
    }
    bool isGlobal() const { return global; }
    const std::shared_ptr<std::vector<std::string>> &
    getParameterIdentifiers() const {
        return parameters;
    }
    const std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> &
    getFunctionBody() const {
        return functionBody;
    }
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        functionBody) {
        this->functionBody = functionBody;
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
                   std::shared_ptr<AST::Type> type,
                   std::shared_ptr<AST::StaticInit> staticInit)
        : identifier(identifier), global(global), type(type),
          staticInit(staticInit) {}
    const std::string &getIdentifier() const { return identifier; }
    bool isGlobal() const { return global; }
    std::shared_ptr<AST::Type> getType() const { return type; }
    std::shared_ptr<AST::StaticInit> getStaticInit() const {
        return staticInit;
    }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels;

  public:
    Program(std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels)
        : topLevels(topLevels) {}
    const std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> &
    getTopLevels() const {
        return topLevels;
    }
};
} // namespace IR

#endif // MIDEND_IR_H
