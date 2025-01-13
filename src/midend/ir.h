#ifndef MIDEND_IR_H
#define MIDEND_IR_H

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
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

// Note: The logical-and and logical-or operators in the AST are NOT binary
// operators in the IR.
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
    int value;

  public:
    ConstantValue(int value) : value(value) {}
    int getValue() { return value; }
    void setValue(int value) { this->value = value; }
};

class VariableValue : public Value {
  private:
    std::string identifier;

  public:
    VariableValue(std::string identifier) : identifier(identifier) {}
    std::string getIdentifier() { return identifier; }
    void setIdentifier(std::string identifier) {
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
    std::shared_ptr<Value> getReturnValue() { return returnValue; }
    void setReturnValue(std::shared_ptr<Value> returnValue) {
        this->returnValue = returnValue;
    }
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<Value> src, dst;

  public:
    UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                     std::shared_ptr<Value> src, std::shared_ptr<Value> dst)
        : unaryOperator(unaryOperator), src(src), dst(dst) {}
    std::shared_ptr<UnaryOperator> getUnaryOperator() { return unaryOperator; }
    std::shared_ptr<Value> getSrc() { return src; }
    std::shared_ptr<Value> getDst() { return dst; }
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
    std::shared_ptr<BinaryOperator> getBinaryOperator() {
        return binaryOperator;
    }
    std::shared_ptr<Value> getSrc1() { return src1; }
    std::shared_ptr<Value> getSrc2() { return src2; }
    std::shared_ptr<Value> getDst() { return dst; }
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
    std::shared_ptr<Value> getSrc() { return src; }
    std::shared_ptr<Value> getDst() { return dst; }
    void setSrc(std::shared_ptr<Value> src) { this->src = src; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class JumpInstruction : public Instruction {
  private:
    std::string target;

  public:
    JumpInstruction(std::string target) : target(target) {}
    std::string getTarget() { return target; }
    void setTarget(std::string target) { this->target = target; }
};

class JumpIfZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfZeroInstruction(std::shared_ptr<Value> condition, std::string target)
        : condition(condition), target(target) {}
    std::shared_ptr<Value> getCondition() { return condition; }
    std::string getTarget() { return target; }
    void setCondition(std::shared_ptr<Value> condition) {
        this->condition = condition;
    }
    void setTarget(std::string target) { this->target = target; }
};

class JumpIfNotZeroInstruction : public Instruction {
  private:
    std::shared_ptr<Value> condition;
    std::string target;

  public:
    JumpIfNotZeroInstruction(std::shared_ptr<Value> condition,
                             std::string target)
        : condition(condition), target(target) {}
    std::shared_ptr<Value> getCondition() { return condition; }
    std::string getTarget() { return target; }
    void setCondition(std::shared_ptr<Value> condition) {
        this->condition = condition;
    }
    void setTarget(std::string target) { this->target = target; }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    LabelInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string label) { this->label = label; }
};

class FunctionCallInstruction : public Instruction {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::shared_ptr<Value>>> args;
    std::shared_ptr<Value> dst;

  public:
    FunctionCallInstruction(
        std::string functionIdentifier,
        std::shared_ptr<std::vector<std::shared_ptr<Value>>> args,
        std::shared_ptr<Value> dst)
        : functionIdentifier(functionIdentifier), args(args), dst(dst) {}
    std::string getFunctionIdentifier() { return functionIdentifier; }
    std::shared_ptr<std::vector<std::shared_ptr<Value>>> getArgs() {
        return args;
    }
    std::shared_ptr<Value> getDst() { return dst; }
    void setFunctionIdentifier(std::string functionIdentifier) {
        this->functionIdentifier = functionIdentifier;
    }
    void setArgs(std::shared_ptr<std::vector<std::shared_ptr<Value>>> args) {
        this->args = args;
    }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
};

class FunctionDefinition {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::string>> parameters;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;

  public:
    FunctionDefinition(
        std::string functionIdentifier,
        std::shared_ptr<std::vector<std::string>> parameters,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody)
        : functionIdentifier(functionIdentifier), parameters(parameters),
          functionBody(functionBody) {}
    std::string getFunctionIdentifier() { return functionIdentifier; }
    std::shared_ptr<std::vector<std::string>> getParameters() {
        return parameters;
    }
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
    getFunctionBody() {
        return functionBody;
    }
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        functionBody) {
        this->functionBody = functionBody;
    }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
        functionDefinitions;

  public:
    Program(std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
                functionDefinitions)
        : functionDefinitions(functionDefinitions) {}
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
    getFunctionDefinitions() {
        return functionDefinitions;
    }
};
} // namespace IR

#endif // MIDEND_IR_H
