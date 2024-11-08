

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

class R10 : public Register {};

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

class Operator {
  public:
    virtual ~Operator() = default;
};

class UnaryOperator : public Operator {};

class NegateOperator : public UnaryOperator {};

class NotOperator : public UnaryOperator {};

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
    void setSrc(std::shared_ptr<Operand> newSrc) { src = newSrc; }
    void setDst(std::shared_ptr<Operand> newDst) { dst = newDst; }
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
    void setUnaryOperator(std::shared_ptr<UnaryOperator> newUnaryOperator) {
        unaryOperator = newUnaryOperator;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        operand = newOperand;
    }
};

class AllocateStack : public Instruction {
  private:
    int addressGivenOffsetFromRBP;

  public:
    AllocateStack(int addressGivenOffsetFromRBP)
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
};
} // namespace Assembly

#endif // ASSEMBLY_H
