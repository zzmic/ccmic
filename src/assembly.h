#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace Assembly {
class Register {
  public:
    virtual ~Register() = default;
};

class AX : public Register {};

class DX : public Register {};

class R10 : public Register {};

class R11 : public Register {};

class Operand {
  public:
    virtual ~Operand() = default;
    virtual int getImmediate() const {
        throw std::runtime_error("Operand is not an immediate");
    };
    virtual std::string getRegister() const {
        throw std::runtime_error("Operand is not a register");
    };
    virtual std::string getPseudoRegister() const {
        throw std::runtime_error("Operand is not a pseudo register");
    };
    virtual int getOffset() const {
        throw std::runtime_error("Operand is not a stack (operand)");
    };
};

class ImmediateOperand : public Operand {
  private:
    int imm;

  public:
    ImmediateOperand(int imm) : imm(imm) {}
    int getImmediate() const override { return imm; }
};

class RegisterOperand : public Operand {
  private:
    std::string reg;

  public:
    RegisterOperand(std::string reg) : reg(reg) {}
    std::string getRegister() const override { return reg; }
};

class PseudoRegisterOperand : public Operand {
  private:
    std::string pseudoReg;

  public:
    PseudoRegisterOperand(std::string pseudoReg) : pseudoReg(pseudoReg) {}
    std::string getPseudoRegister() const override { return pseudoReg; }
};

class StackOperand : public Operand {
  private:
    int offset;

  public:
    StackOperand(int offset) : offset(offset) {}
    int getOffset() const override { return offset; }
};

class CondCode {
  public:
    virtual ~CondCode() = default;
};

class E : public CondCode {};

class NE : public CondCode {};

class G : public CondCode {};

class GE : public CondCode {};

class L : public CondCode {};

class LE : public CondCode {};

class Operator {
  public:
    virtual ~Operator() = default;
};

class UnaryOperator : public Operator {};

class NegateOperator : public UnaryOperator {};

class ComplementOperator : public UnaryOperator {};

class NotOperator : public UnaryOperator {};

class BinaryOperator : public Operator {};

class AddOperator : public BinaryOperator {};

class SubtractOperator : public BinaryOperator {};

class MultiplyOperator : public BinaryOperator {};

class Instruction {
  public:
    virtual ~Instruction() = default;
};

class MovInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> src, dst;

  public:
    MovInstruction(std::shared_ptr<Operand> src, std::shared_ptr<Operand> dst)
        : src(src), dst(dst) {}
    std::shared_ptr<Operand> getSrc() { return src; }
    std::shared_ptr<Operand> getDst() { return dst; }
    void setSrc(std::shared_ptr<Operand> src) { this->src = src; }
    void setDst(std::shared_ptr<Operand> dst) { this->dst = dst; }
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<Operand> operand;

  public:
    UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                     std::shared_ptr<Operand> operand)
        : unaryOperator(unaryOperator), operand(operand) {}
    std::shared_ptr<UnaryOperator> getUnaryOperator() { return unaryOperator; }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setUnaryOperator(std::shared_ptr<UnaryOperator> unaryOperator) {
        this->unaryOperator = unaryOperator;
    }
    void setOperand(std::shared_ptr<Operand> operand) {
        this->operand = operand;
    }
};

class BinaryInstruction : public Instruction {
  private:
    std::shared_ptr<BinaryOperator> binaryOperator;
    std::shared_ptr<Operand> operand1, operand2;

  public:
    BinaryInstruction(std::shared_ptr<BinaryOperator> binaryOperator,
                      std::shared_ptr<Operand> operand1,
                      std::shared_ptr<Operand> operand2)
        : binaryOperator(binaryOperator), operand1(operand1),
          operand2(operand2) {}
    std::shared_ptr<BinaryOperator> getBinaryOperator() {
        return binaryOperator;
    }
    std::shared_ptr<Operand> getOperand1() { return operand1; }
    std::shared_ptr<Operand> getOperand2() { return operand2; }
    void setBinaryOperator(std::shared_ptr<BinaryOperator> binaryOperator) {
        this->binaryOperator = binaryOperator;
    }
    void setOperand1(std::shared_ptr<Operand> operand1) {
        this->operand1 = operand1;
    }
    void setOperand2(std::shared_ptr<Operand> operand2) {
        this->operand2 = operand2;
    }
};

class CmpInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand1, operand2;

  public:
    CmpInstruction(std::shared_ptr<Operand> operand1,
                   std::shared_ptr<Operand> operand2)
        : operand1(operand1), operand2(operand2) {}
    std::shared_ptr<Operand> getOperand1() { return operand1; }
    std::shared_ptr<Operand> getOperand2() { return operand2; }
    void setOperand1(std::shared_ptr<Operand> operand1) {
        this->operand1 = operand1;
    }
    void setOperand2(std::shared_ptr<Operand> operand2) {
        this->operand2 = operand2;
    }
};

class IdivInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand;

  public:
    IdivInstruction(std::shared_ptr<Operand> operand) : operand(operand) {}
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setOperand(std::shared_ptr<Operand> operand) {
        this->operand = operand;
    }
};

class CdqInstruction : public Instruction {};

class JmpInstruction : public Instruction {
  private:
    std::string label;

  public:
    JmpInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string label) { this->label = label; }
};

class JmpCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::string label;

  public:
    JmpCCInstruction(std::shared_ptr<CondCode> condCode, std::string label)
        : condCode(condCode), label(label) {}
    std::shared_ptr<CondCode> getCondCode() { return condCode; }
    std::string getLabel() { return label; }
    void setCondCode(std::shared_ptr<CondCode> condCode) {
        this->condCode = condCode;
    }
    void setLabel(std::string label) { this->label = label; }
};

class SetCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::shared_ptr<Operand> operand;

  public:
    SetCCInstruction(std::shared_ptr<CondCode> condCode,
                     std::shared_ptr<Operand> operand)
        : condCode(condCode), operand(operand) {}
    std::shared_ptr<CondCode> getCondCode() { return condCode; }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setCondCode(std::shared_ptr<CondCode> condCode) {
        this->condCode = condCode;
    }
    void setOperand(std::shared_ptr<Operand> operand) {
        this->operand = operand;
    }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    LabelInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string label) { this->label = label; }
};

class AllocateStackInstruction : public Instruction {
  private:
    int addressGivenOffsetFromRBP;

  public:
    AllocateStackInstruction(int addressGivenOffsetFromRBP)
        : addressGivenOffsetFromRBP(addressGivenOffsetFromRBP) {}
    int getAddressGivenOffsetFromRBP() { return addressGivenOffsetFromRBP; }
};

class RetInstruction : public Instruction {};

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
    void setFunctionDefinition(
        std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
            newFunctionDefinition) {
        functionDefinition = newFunctionDefinition;
    }
};
} // namespace Assembly

#endif // ASSEMBLY_H
