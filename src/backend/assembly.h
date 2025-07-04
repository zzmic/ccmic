#ifndef BACKEND_ASSEMBLY_H
#define BACKEND_ASSEMBLY_H

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
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

class ReservedRegister : public Register {};

class SP : public ReservedRegister {};

class BP : public ReservedRegister {};

class Operand {
  public:
    virtual ~Operand() = default;
    virtual int getImmediate() const {
        throw std::logic_error("Operand is not an immediate");
    };
    virtual std::shared_ptr<Register> getRegister() const {
        throw std::logic_error("Operand is not a register");
    };
    virtual std::shared_ptr<ReservedRegister> getReservedRegister() const {
        throw std::logic_error("Operand is not a reserved register");
    };
    virtual std::string getPseudoRegister() const {
        throw std::logic_error("Operand is not a pseudo register");
    };
    virtual int getOffset() const {
        throw std::logic_error("Operand is not a stack (operand)");
    };
    virtual std::string getIdentifier() const {
        throw std::logic_error("Operand is not a data (operand)");
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
    std::unordered_map<int, std::unordered_map<std::type_index, std::string>>
        regMappings = {{1, // 1-byte registers.
                        {{typeid(AX), "%al"},
                         {typeid(CX), "%cl"},
                         {typeid(DX), "%dl"},
                         {typeid(DI), "%dil"},
                         {typeid(SI), "%sil"},
                         {typeid(R8), "%r8b"},
                         {typeid(R9), "%r9b"},
                         {typeid(R10), "%r10b"},
                         {typeid(R11), "%r11b"}}},
                       {4, // 4-byte registers.
                        {{typeid(AX), "%eax"},
                         {typeid(CX), "%ecx"},
                         {typeid(DX), "%edx"},
                         {typeid(DI), "%edi"},
                         {typeid(SI), "%esi"},
                         {typeid(R8), "%r8d"},
                         {typeid(R9), "%r9d"},
                         {typeid(R10), "%r10d"},
                         {typeid(R11), "%r11d"}}},
                       {8, // 8-byte registers.
                        {{typeid(AX), "%rax"},
                         {typeid(CX), "%rcx"},
                         {typeid(DX), "%rdx"},
                         {typeid(DI), "%rdi"},
                         {typeid(SI), "%rsi"},
                         {typeid(R8), "%r8"},
                         {typeid(R9), "%r9"},
                         {typeid(R10), "%r10"},
                         {typeid(R11), "%r11"}}}};

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
            throw std::logic_error("Unsupported register");
        }
    }
    std::shared_ptr<Register> getRegister() const override { return reg; }
    std::string getRegisterInBytesInStr(int size) const {
        auto sizeIt = regMappings.find(size);
        if (sizeIt == regMappings.end()) {
            throw std::logic_error("Unsupported register size");
        }
        const auto &sizeMappings = sizeIt->second;
        auto &r = *reg.get();
        auto regIt = sizeMappings.find(typeid(r));
        if (regIt == sizeMappings.end()) {
            throw std::logic_error("Unsupported register");
        }
        return regIt->second;
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
    std::shared_ptr<ReservedRegister> reservedReg;

  public:
    StackOperand(int offset, std::shared_ptr<ReservedRegister> reservedReg)
        : offset(offset), reservedReg(reservedReg) {}
    int getOffset() const override { return offset; }
    std::shared_ptr<ReservedRegister> getReservedRegister() const override {
        return reservedReg;
    }
    std::string getReservedRegisterInStr() const {
        if (auto sp = std::dynamic_pointer_cast<SP>(reservedReg)) {
            return "%rsp";
        }
        else if (auto bp = std::dynamic_pointer_cast<BP>(reservedReg)) {
            return "%rbp";
        }
        else {
            throw std::logic_error("Unsupported reserved register");
        }
    }
};

class DataOperand : public Operand {
  private:
    std::string identifier;

  public:
    DataOperand(std::string identifier) : identifier(identifier) {}
    std::string getIdentifier() const override { return identifier; }
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
    void setSrc(std::shared_ptr<Operand> newSrc) { this->src = newSrc; }
    void setDst(std::shared_ptr<Operand> newDst) { this->dst = newDst; }
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
        this->unaryOperator = newUnaryOperator;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        this->operand = newOperand;
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
    void setBinaryOperator(std::shared_ptr<BinaryOperator> newBinaryOperator) {
        this->binaryOperator = newBinaryOperator;
    }
    void setOperand1(std::shared_ptr<Operand> newOperand1) {
        this->operand1 = newOperand1;
    }
    void setOperand2(std::shared_ptr<Operand> newOperand2) {
        this->operand2 = newOperand2;
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
    void setOperand1(std::shared_ptr<Operand> newOperand1) {
        this->operand1 = newOperand1;
    }
    void setOperand2(std::shared_ptr<Operand> newOperand2) {
        this->operand2 = newOperand2;
    }
};

class IdivInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand;

  public:
    IdivInstruction(std::shared_ptr<Operand> operand) : operand(operand) {}
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        this->operand = newOperand;
    }
};

class CdqInstruction : public Instruction {};

class JmpInstruction : public Instruction {
  private:
    std::string label;

  public:
    JmpInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string newLabel) { this->label = newLabel; }
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
    void setCondCode(std::shared_ptr<CondCode> newCondCode) {
        this->condCode = newCondCode;
    }
    void setLabel(std::string newLabel) { this->label = newLabel; }
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
    void setCondCode(std::shared_ptr<CondCode> newCondCode) {
        this->condCode = newCondCode;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        this->operand = newOperand;
    }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    LabelInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string newLabel) { this->label = newLabel; }
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
    void setOperand(std::shared_ptr<Operand> newOperand) {
        this->operand = newOperand;
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

class TopLevel {
  public:
    virtual ~TopLevel() = default;
};

class FunctionDefinition : public TopLevel {
  private:
    std::string functionIdentifier;
    bool global;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;
    size_t stackSize;

  public:
    FunctionDefinition(
        std::string functionIdentifier, bool global,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody,
        size_t stackSize)
        : functionIdentifier(functionIdentifier), global(global),
          functionBody(functionBody), stackSize(stackSize) {}
    std::string getFunctionIdentifier() { return functionIdentifier; }
    bool isGlobal() { return global; }
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
    getFunctionBody() {
        return functionBody;
    }
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        newFunctionBody) {
        this->functionBody = newFunctionBody;
    }
    size_t getStackSize() { return stackSize; }
    void setStackSize(size_t newStackSize) { this->stackSize = newStackSize; }
};

class StaticVariable : public TopLevel {
  private:
    std::string identifier;
    bool global;
    int initialValue;

  public:
    StaticVariable(std::string identifier, bool global, int initialValue)
        : identifier(identifier), global(global), initialValue(initialValue) {}
    std::string getIdentifier() { return identifier; }
    bool isGlobal() { return global; }
    int getInitialValue() { return initialValue; }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels;

  public:
    Program(std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels)
        : topLevels(topLevels) {}
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> getTopLevels() {
        return topLevels;
    }
    void setTopLevels(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> newTopLevels) {
        this->topLevels = newTopLevels;
    }
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_H
