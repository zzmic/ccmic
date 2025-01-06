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

class CX : public Register {};

class DX : public Register {};

class DI : public Register {};

class SI : public Register {};

class R8 : public Register {};

class R9 : public Register {};

class R10 : public Register {};

class R11 : public Register {};

class Operand {
  public:
    virtual ~Operand() = default;
    virtual int getImmediate() const {
        throw std::runtime_error("Operand is not an immediate");
    };
    virtual std::shared_ptr<Register> getRegister() const {
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
    std::shared_ptr<Register> reg;

  public:
    RegisterOperand(std::shared_ptr<Register> reg) : reg(reg) {}
    RegisterOperand(std::string regInStr) {
        if (regInStr == "AX") {
            reg = std::make_shared<AX>();
        }
        else if (regInStr == "CX") {
            reg = std::make_shared<CX>();
        }
        else if (regInStr == "DX") {
            reg = std::make_shared<DX>();
        }
        else if (regInStr == "DI") {
            reg = std::make_shared<DI>();
        }
        else if (regInStr == "SI") {
            reg = std::make_shared<SI>();
        }
        else if (regInStr == "R8") {
            reg = std::make_shared<R8>();
        }
        else if (regInStr == "R9") {
            reg = std::make_shared<R9>();
        }
        else if (regInStr == "R10") {
            reg = std::make_shared<R10>();
        }
        else if (regInStr == "R11") {
            reg = std::make_shared<R11>();
        }
        else {
            throw std::runtime_error("Unsupported register");
        }
    }
    std::shared_ptr<Register> getRegister() const override { return reg; }
    std::string getRegisterInStr() const {
        if (std::dynamic_pointer_cast<AX>(reg)) {
            return "%eax";
        }
        else if (std::dynamic_pointer_cast<CX>(reg)) {
            return "%ecx";
        }
        else if (std::dynamic_pointer_cast<DX>(reg)) {
            return "%edx";
        }
        else if (std::dynamic_pointer_cast<DI>(reg)) {
            return "%edi";
        }
        else if (std::dynamic_pointer_cast<SI>(reg)) {
            return "%esi";
        }
        else if (std::dynamic_pointer_cast<R8>(reg)) {
            return "%r8d";
        }
        else if (std::dynamic_pointer_cast<R9>(reg)) {
            return "%r9d";
        }
        else if (std::dynamic_pointer_cast<R10>(reg)) {
            return "%r10d";
        }
        else if (std::dynamic_pointer_cast<R11>(reg)) {
            return "%r11d";
        }
        else {
            throw std::runtime_error("Unsupported register");
        }
    }
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

class DeallocateStackInstruction : public Instruction {
  private:
    int addressGivenOffsetFromRBP;

  public:
    DeallocateStackInstruction(int addressGivenOffsetFromRBP)
        : addressGivenOffsetFromRBP(addressGivenOffsetFromRBP) {}
    int getAddressGivenOffsetFromRBP() { return addressGivenOffsetFromRBP; }
};

class PushInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand;

  public:
    PushInstruction(std::shared_ptr<Operand> operand) : operand(operand) {}
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setOperand(std::shared_ptr<Operand> operand) {
        this->operand = operand;
    }
};

class CallInstruction : public Instruction {
  private:
    std::string functionIdentifier;

  public:
    CallInstruction(std::string functionIdentifier)
        : functionIdentifier(functionIdentifier) {}
    std::string getFunctionIdentifier() { return functionIdentifier; }
};

class RetInstruction : public Instruction {};

class FunctionDefinition {
  private:
    std::string functionIdentifier;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;
    std::size_t stackSize;

  public:
    FunctionDefinition(
        std::string functionIdentifier,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody)
        : functionIdentifier(functionIdentifier), functionBody(functionBody),
          stackSize(0) {}
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
    std::size_t getStackSize() { return stackSize; }
    void setStackSize(std::size_t stackSize) { this->stackSize = stackSize; }
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
    void setFunctionDefinitions(
        std::shared_ptr<std::vector<std::shared_ptr<FunctionDefinition>>>
            functionDefinitions) {
        this->functionDefinitions = functionDefinitions;
    }
};
} // namespace Assembly

#endif // ASSEMBLY_H
