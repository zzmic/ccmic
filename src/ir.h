#ifndef IR_H
#define IR_H

#include <memory>
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

class ComplementOperator : public UnaryOperator {};

class NegateOperator : public UnaryOperator {};

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
    void setValue(int newValue) { value = newValue; }
};

class VariableValue : public Value {
  private:
    std::string identifier;

  public:
    VariableValue(std::string identifier) : identifier(identifier) {}
    std::string getIdentifier() { return identifier; }
    void setIdentifier(std::string newIdentifier) {
        identifier = newIdentifier;
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
    void setReturnValue(std::shared_ptr<Value> newReturnValue) {
        returnValue = newReturnValue;
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
    void setUnaryOperator(std::shared_ptr<UnaryOperator> newUnaryOperator) {
        unaryOperator = newUnaryOperator;
    }
    void setSrc(std::shared_ptr<Value> newSrc) { src = newSrc; }
    void setDst(std::shared_ptr<Value> newDst) { dst = newDst; }
};

class FunctionDefinition {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;

  public:
    FunctionDefinition(const std::string functionIdentifier)
        : functionIdentifier(functionIdentifier) {}
    std::string getFunctionIdentifier() { return functionIdentifier; }
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
    getFunctionBody() {
        return functionBody;
    }
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        newFunctionBody) {
        functionBody = newFunctionBody;
    }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
        functionDefinition;

  public:
    Program(std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
                functionDefinition)
        : functionDefinition(functionDefinition) {}
    std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
    getFunctionDefinition() {
        return functionDefinition;
    }
};
} // namespace IR

#endif // IR_H
