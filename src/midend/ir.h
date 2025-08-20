#ifndef MIDEND_IR_H
#define MIDEND_IR_H

#include "../frontend/semanticAnalysisPasses.h"
#include "../frontend/type.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
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

/*
 * Note: The logical-and and logical-or operators in the AST are NOT binary
 * operators in the IR.
 */
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
    std::unique_ptr<AST::Constant> astConstant;

  public:
    explicit ConstantValue(std::unique_ptr<AST::Constant> astConstant);
    ~ConstantValue();
    [[nodiscard]] std::unique_ptr<AST::Constant> &getASTConstant();
    void setASTConstant(std::unique_ptr<AST::Constant> newAstConstant);
};

class VariableValue : public Value {
  private:
    std::string identifier;

  public:
    explicit VariableValue(std::string_view identifier);
    ~VariableValue();
    [[nodiscard]] const std::string &getIdentifier() const;
    void setIdentifier(std::string_view newIdentifier);
};

class Instruction {
  public:
    virtual ~Instruction() = default;
};

class ReturnInstruction : public Instruction {
  private:
    std::unique_ptr<Value> returnValue;

  public:
    explicit ReturnInstruction(std::unique_ptr<Value> returnValue);
    [[nodiscard]] std::unique_ptr<Value> &getReturnValue();
    void setReturnValue(std::unique_ptr<Value> newReturnValue);
};

class SignExtendInstruction : public Instruction {
  private:
    std::unique_ptr<Value> src, dst;

  public:
    SignExtendInstruction(std::unique_ptr<Value> src,
                          std::unique_ptr<Value> dst);
    [[nodiscard]] std::unique_ptr<Value> &getSrc();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setSrc(std::unique_ptr<Value> newSrc);
    void setDst(std::unique_ptr<Value> newDst);
};

class TruncateInstruction : public Instruction {
  private:
    std::unique_ptr<Value> src, dst;

  public:
    TruncateInstruction(std::unique_ptr<Value> src, std::unique_ptr<Value> dst);
    [[nodiscard]] std::unique_ptr<Value> &getSrc();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setSrc(std::unique_ptr<Value> newSrc);
    void setDst(std::unique_ptr<Value> newDst);
};

class UnaryInstruction : public Instruction {
  private:
    std::unique_ptr<UnaryOperator> unaryOperator;
    std::unique_ptr<Value> src, dst;

  public:
    UnaryInstruction(std::unique_ptr<UnaryOperator> unaryOperator,
                     std::unique_ptr<Value> src, std::unique_ptr<Value> dst);
    [[nodiscard]] std::unique_ptr<UnaryOperator> &getUnaryOperator();
    [[nodiscard]] std::unique_ptr<Value> &getSrc();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setUnaryOperator(std::unique_ptr<UnaryOperator> newUnaryOperator);
    void setSrc(std::unique_ptr<Value> newSrc);
    void setDst(std::unique_ptr<Value> newDst);
};

class BinaryInstruction : public Instruction {
  private:
    std::unique_ptr<BinaryOperator> binaryOperator;
    std::unique_ptr<Value> src1, src2, dst;

  public:
    BinaryInstruction(std::unique_ptr<BinaryOperator> binaryOperator,
                      std::unique_ptr<Value> src1, std::unique_ptr<Value> src2,
                      std::unique_ptr<Value> dst);
    [[nodiscard]] std::unique_ptr<BinaryOperator> &getBinaryOperator();
    [[nodiscard]] std::unique_ptr<Value> &getSrc1();
    [[nodiscard]] std::unique_ptr<Value> &getSrc2();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setBinaryOperator(std::unique_ptr<BinaryOperator> newBinaryOperator);
    void setSrc1(std::unique_ptr<Value> newSrc1);
    void setSrc2(std::unique_ptr<Value> newSrc2);
    void setDst(std::unique_ptr<Value> newDst);
};

class CopyInstruction : public Instruction {
  private:
    std::unique_ptr<Value> src, dst;

  public:
    CopyInstruction(std::unique_ptr<Value> src, std::unique_ptr<Value> dst);
    [[nodiscard]] std::unique_ptr<Value> &getSrc();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setSrc(std::unique_ptr<Value> newSrc);
    void setDst(std::unique_ptr<Value> newDst);
};

class JumpInstruction : public Instruction {
  private:
    std::string target;

  public:
    explicit JumpInstruction(std::string_view target);
    [[nodiscard]] const std::string &getTarget() const;
    void setTarget(std::string_view newTarget);
};

class JumpIfZeroInstruction : public Instruction {
  private:
    std::unique_ptr<Value> condition;
    std::string target;

  public:
    JumpIfZeroInstruction(std::unique_ptr<Value> condition,
                          std::string_view target);
    [[nodiscard]] std::unique_ptr<Value> &getCondition();
    [[nodiscard]] const std::string &getTarget() const;
    void setCondition(std::unique_ptr<Value> newCondition);
    void setTarget(std::string_view newTarget);
};

class JumpIfNotZeroInstruction : public Instruction {
  private:
    std::unique_ptr<Value> condition;
    std::string target;

  public:
    JumpIfNotZeroInstruction(std::unique_ptr<Value> condition,
                             std::string_view target);
    [[nodiscard]] std::unique_ptr<Value> &getCondition();
    [[nodiscard]] const std::string &getTarget() const;
    void setCondition(std::unique_ptr<Value> newCondition);
    void setTarget(std::string_view newTarget);
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit LabelInstruction(std::string_view label);
    [[nodiscard]] const std::string &getLabel() const;
    void setLabel(std::string_view newLabel);
};

class FunctionCallInstruction : public Instruction {
  private:
    std::string functionIdentifier;
    std::vector<std::unique_ptr<Value>> args;
    std::unique_ptr<Value> dst;

  public:
    FunctionCallInstruction(std::string_view functionIdentifier,
                            std::vector<std::unique_ptr<Value>> args,
                            std::unique_ptr<Value> dst);
    [[nodiscard]] const std::string &getFunctionIdentifier() const;
    [[nodiscard]] std::vector<std::unique_ptr<Value>> &getArgs();
    [[nodiscard]] std::unique_ptr<Value> &getDst();
    void setFunctionIdentifier(std::string_view newFunctionIdentifier);
    void setArgs(std::vector<std::unique_ptr<Value>> newArgs);
    void setDst(std::unique_ptr<Value> newDst);
};

class TopLevel {
  public:
    virtual ~TopLevel() = default;
};

class FunctionDefinition : public TopLevel {
  private:
    std::string functionIdentifier;
    bool global;
    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<Instruction>> functionBody;

  public:
    FunctionDefinition(std::string_view functionIdentifier, bool global,
                       std::vector<std::string> parameters,
                       std::vector<std::unique_ptr<Instruction>> functionBody);
    [[nodiscard]] const std::string &getFunctionIdentifier() const;
    [[nodiscard]] bool isGlobal() const;
    [[nodiscard]] std::vector<std::string> &getParameterIdentifiers();
    [[nodiscard]] std::vector<std::unique_ptr<Instruction>> &getFunctionBody();
    void
    setFunctionBody(std::vector<std::unique_ptr<Instruction>> newFunctionBody);
};

class StaticVariable : public TopLevel {
  private:
    std::string identifier;
    bool global;
    std::unique_ptr<AST::Type> type;
    std::unique_ptr<AST::StaticInit> staticInit;

  public:
    StaticVariable(std::string_view identifier, bool global,
                   std::unique_ptr<AST::Type> type,
                   std::unique_ptr<AST::StaticInit> staticInit);
    [[nodiscard]] const std::string &getIdentifier() const;
    [[nodiscard]] bool isGlobal() const;
    [[nodiscard]] std::unique_ptr<AST::Type> &getType();
    [[nodiscard]] std::unique_ptr<AST::StaticInit> &getStaticInit();
};

class Program {
  private:
    std::vector<std::unique_ptr<TopLevel>> topLevels;

  public:
    explicit Program(std::vector<std::unique_ptr<TopLevel>> topLevels);
    [[nodiscard]] std::vector<std::unique_ptr<TopLevel>> &getTopLevels();
};
} // namespace IR

#endif // MIDEND_IR_H
