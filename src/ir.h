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

class BinaryOperator : public Operator {};

class AddOperator : public BinaryOperator {};

class SubtractOperator : public BinaryOperator {};

class MultiplyOperator : public BinaryOperator {};

class DivideOperator : public BinaryOperator {};

class RemainderOperator : public BinaryOperator {};

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
    std::shared_ptr<Value> lhs, rhs, dst;

  public:
    BinaryInstruction(std::shared_ptr<BinaryOperator> binaryOperator,
                      std::shared_ptr<Value> lhs, std::shared_ptr<Value> rhs,
                      std::shared_ptr<Value> dst)
        : binaryOperator(binaryOperator), lhs(lhs), rhs(rhs), dst(dst) {}
    std::shared_ptr<BinaryOperator> getBinaryOperator() {
        return binaryOperator;
    }
    std::shared_ptr<Value> getLhs() { return lhs; }
    std::shared_ptr<Value> getRhs() { return rhs; }
    std::shared_ptr<Value> getDst() { return dst; }
    void setBinaryOperator(std::shared_ptr<BinaryOperator> binaryOperator) {
        this->binaryOperator = binaryOperator;
    }
    void setLhs(std::shared_ptr<Value> lhs) { this->lhs = lhs; }
    void setRhs(std::shared_ptr<Value> rhs) { this->rhs = rhs; }
    void setDst(std::shared_ptr<Value> dst) { this->dst = dst; }
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
                        functionBody) {
        this->functionBody = functionBody;
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
