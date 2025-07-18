#ifndef BACKEND_ASSEMBLY_H
#define BACKEND_ASSEMBLY_H

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../frontend/semanticAnalysisPasses.h"

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
    explicit ImmediateOperand(int imm) : imm(imm) {}
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
                         {typeid(R11), "%r11b"},
                         {typeid(SP), "%spl"},
                         {typeid(BP), "%bpl"}}},
                       {4, // 4-byte registers.
                        {{typeid(AX), "%eax"},
                         {typeid(CX), "%ecx"},
                         {typeid(DX), "%edx"},
                         {typeid(DI), "%edi"},
                         {typeid(SI), "%esi"},
                         {typeid(R8), "%r8d"},
                         {typeid(R9), "%r9d"},
                         {typeid(R10), "%r10d"},
                         {typeid(R11), "%r11d"},
                         {typeid(SP), "%esp"},
                         {typeid(BP), "%ebp"}}},
                       {8, // 8-byte registers.
                        {{typeid(AX), "%rax"},
                         {typeid(CX), "%rcx"},
                         {typeid(DX), "%rdx"},
                         {typeid(DI), "%rdi"},
                         {typeid(SI), "%rsi"},
                         {typeid(R8), "%r8"},
                         {typeid(R9), "%r9"},
                         {typeid(R10), "%r10"},
                         {typeid(R11), "%r11"},
                         {typeid(SP), "%rsp"},
                         {typeid(BP), "%rbp"}}}};

  public:
    explicit RegisterOperand(std::shared_ptr<Register> reg) : reg(reg) {
        if (!reg) {
            throw std::logic_error("Creating RegisterOperand with null reg");
        }
    }
    explicit RegisterOperand(std::string regInStr) {
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
        else if (regInStr == "RSP") {
            reg = std::make_shared<SP>();
        }
        else {
            throw std::logic_error("Unsupported register while creating "
                                   "RegisterOperand: " +
                                   regInStr);
        }
    }
    std::shared_ptr<Register> getRegister() const override { return reg; }
    std::string getRegisterInBytesInStr(int size) const {
        auto sizeIt = regMappings.find(size);
        if (sizeIt == regMappings.end()) {
            throw std::logic_error("Unsupported register size while getting "
                                   "register in bytes in string: " +
                                   std::to_string(size));
        }
        const auto &sizeMappings = sizeIt->second;
        auto &r = *reg.get();
        auto regIt = sizeMappings.find(typeid(r));
        if (regIt == sizeMappings.end()) {
            throw std::logic_error("Unsupported register type while getting "
                                   "register in bytes in string: " +
                                   std::string(typeid(r).name()));
        }
        return regIt->second;
    }
};

class PseudoRegisterOperand : public Operand {
  private:
    std::string pseudoReg;

  public:
    explicit PseudoRegisterOperand(std::string pseudoReg)
        : pseudoReg(pseudoReg) {}
    std::string getPseudoRegister() const override { return pseudoReg; }
};

class StackOperand : public Operand {
  private:
    int offset;
    std::shared_ptr<ReservedRegister> reservedReg;

  public:
    explicit StackOperand(int offset,
                          std::shared_ptr<ReservedRegister> reservedReg)
        : offset(offset), reservedReg(reservedReg) {
        if (!reservedReg) {
            throw std::logic_error(
                "Creating StackOperand with null reservedReg");
        }
    }
    int getOffset() const override { return offset; }
    std::shared_ptr<ReservedRegister> getReservedRegister() const override {
        return reservedReg;
    }
    std::string getReservedRegisterInStr() const {
        if (auto sp = std::dynamic_pointer_cast<SP>(reservedReg)) {
            return "%rsp";
        }
        else if (auto rsp = std::dynamic_pointer_cast<SP>(reservedReg)) {
            return "%rsp";
        }
        else if (auto bp = std::dynamic_pointer_cast<BP>(reservedReg)) {
            return "%rbp";
        }
        else {
            auto reg = reservedReg.get();
            throw std::logic_error("Unsupported reserved register while "
                                   "getting reserved register in string: " +
                                   std::string(typeid(*reg).name()));
        }
    }
};

class DataOperand : public Operand {
  private:
    std::string identifier;

  public:
    explicit DataOperand(std::string identifier) : identifier(identifier) {}
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

class AssemblyType {
  public:
    virtual ~AssemblyType() = default;
};

class Longword : public AssemblyType {};

class Quadword : public AssemblyType {};

class MovInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> src, dst;

  public:
    explicit MovInstruction(std::shared_ptr<AssemblyType> type,
                            std::shared_ptr<Operand> src,
                            std::shared_ptr<Operand> dst)
        : type(type), src(src), dst(dst) {
        if (!type) {
            throw std::logic_error("Creating MovInstruction with null type");
        }
        if (!src) {
            throw std::logic_error("Creating MovInstruction with null src");
        }
        if (!dst) {
            throw std::logic_error("Creating MovInstruction with null dst");
        }
    }
    std::shared_ptr<AssemblyType> getType() { return type; }
    std::shared_ptr<Operand> getSrc() { return src; }
    std::shared_ptr<Operand> getDst() { return dst; }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in MovInstruction");
        }
        this->type = newType;
    }
    void setSrc(std::shared_ptr<Operand> newSrc) {
        if (!newSrc) {
            throw std::logic_error(
                "Setting null source operand in MovInstruction");
        }
        this->src = newSrc;
    }
    void setDst(std::shared_ptr<Operand> newDst) {
        if (!newDst) {
            throw std::logic_error(
                "Setting null destination operand in MovInstruction");
        }
        this->dst = newDst;
    }
};

class MovsxInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> src, dst;

  public:
    explicit MovsxInstruction(std::shared_ptr<Operand> src,
                              std::shared_ptr<Operand> dst)
        : src(src), dst(dst) {
        if (!src) {
            throw std::logic_error("Creating MovsxInstruction with null src");
        }
        if (!dst) {
            throw std::logic_error("Creating MovsxInstruction with null dst");
        }
    }
    std::shared_ptr<Operand> getSrc() { return src; }
    std::shared_ptr<Operand> getDst() { return dst; }
    void setSrc(std::shared_ptr<Operand> newSrc) {
        if (!newSrc) {
            throw std::logic_error(
                "Setting null source operand in MovsxInstruction");
        }
        this->src = newSrc;
    }
    void setDst(std::shared_ptr<Operand> newDst) {
        if (!newDst) {
            throw std::logic_error(
                "Setting null destination operand in MovsxInstruction");
        }
        this->dst = newDst;
    }
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand;

  public:
    explicit UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                              std::shared_ptr<AssemblyType> type,
                              std::shared_ptr<Operand> operand)
        : unaryOperator(unaryOperator), type(type), operand(operand) {
        if (!unaryOperator) {
            throw std::logic_error(
                "Creating UnaryInstruction with null unaryOperator");
        }
        if (!type) {
            throw std::logic_error("Creating UnaryInstruction with null type");
        }
        if (!operand) {
            throw std::logic_error(
                "Creating UnaryInstruction with null operand");
        }
    }
    std::shared_ptr<UnaryOperator> getUnaryOperator() { return unaryOperator; }
    std::shared_ptr<AssemblyType> getType() { return type; }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setUnaryOperator(std::shared_ptr<UnaryOperator> newUnaryOperator) {
        if (!newUnaryOperator) {
            throw std::logic_error(
                "Setting null unary operator in UnaryInstruction");
        }
        this->unaryOperator = newUnaryOperator;
    }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in UnaryInstruction");
        }
        this->type = newType;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        if (!newOperand) {
            throw std::logic_error("Setting null operand in UnaryInstruction");
        }
        this->operand = newOperand;
    }
};

class BinaryInstruction : public Instruction {
  private:
    std::shared_ptr<BinaryOperator> binaryOperator;
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand1, operand2;

  public:
    explicit BinaryInstruction(std::shared_ptr<BinaryOperator> binaryOperator,
                               std::shared_ptr<AssemblyType> type,
                               std::shared_ptr<Operand> operand1,
                               std::shared_ptr<Operand> operand2)
        : binaryOperator(binaryOperator), type(type), operand1(operand1),
          operand2(operand2) {
        if (!binaryOperator) {
            throw std::logic_error(
                "Creating BinaryInstruction with null binaryOperator");
        }
        if (!type) {
            throw std::logic_error("Creating BinaryInstruction with null type");
        }
        if (!operand1) {
            throw std::logic_error(
                "Creating BinaryInstruction with null operand1");
        }
        if (!operand2) {
            throw std::logic_error(
                "Creating BinaryInstruction with null operand2");
        }
    }
    std::shared_ptr<BinaryOperator> getBinaryOperator() {
        return binaryOperator;
    }
    std::shared_ptr<AssemblyType> getType() { return type; }
    std::shared_ptr<Operand> getOperand1() { return operand1; }
    std::shared_ptr<Operand> getOperand2() { return operand2; }
    void setBinaryOperator(std::shared_ptr<BinaryOperator> newBinaryOperator) {
        if (!newBinaryOperator) {
            throw std::logic_error(
                "Setting null binary operator in BinaryInstruction");
        }
        this->binaryOperator = newBinaryOperator;
    }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in BinaryInstruction");
        }
        this->type = newType;
    }
    void setOperand1(std::shared_ptr<Operand> newOperand1) {
        if (!newOperand1) {
            throw std::logic_error(
                "Setting null operand1 in BinaryInstruction");
        }
        this->operand1 = newOperand1;
    }
    void setOperand2(std::shared_ptr<Operand> newOperand2) {
        if (!newOperand2) {
            throw std::logic_error(
                "Setting null operand2 in BinaryInstruction");
        }
        this->operand2 = newOperand2;
    }
};

class CmpInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand1, operand2;

  public:
    explicit CmpInstruction(std::shared_ptr<AssemblyType> type,
                            std::shared_ptr<Operand> operand1,
                            std::shared_ptr<Operand> operand2)
        : type(type), operand1(operand1), operand2(operand2) {
        if (!type) {
            throw std::logic_error("Creating CmpInstruction with null type");
        }
        if (!operand1) {
            throw std::logic_error(
                "Creating CmpInstruction with null operand1");
        }
        if (!operand2) {
            throw std::logic_error(
                "Creating CmpInstruction with null operand2");
        }
    }
    std::shared_ptr<AssemblyType> getType() { return type; }
    std::shared_ptr<Operand> getOperand1() { return operand1; }
    std::shared_ptr<Operand> getOperand2() { return operand2; }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in CmpInstruction");
        }
        this->type = newType;
    }
    void setOperand1(std::shared_ptr<Operand> newOperand1) {
        if (!newOperand1) {
            throw std::logic_error("Setting null operand1 in CmpInstruction");
        }
        this->operand1 = newOperand1;
    }
    void setOperand2(std::shared_ptr<Operand> newOperand2) {
        if (!newOperand2) {
            throw std::logic_error("Setting null operand2 in CmpInstruction");
        }
        this->operand2 = newOperand2;
    }
};

class IdivInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand;

  public:
    explicit IdivInstruction(std::shared_ptr<AssemblyType> type,
                             std::shared_ptr<Operand> operand)
        : type(type), operand(operand) {
        if (!type) {
            throw std::logic_error("Creating IdivInstruction with null type");
        }
        if (!operand) {
            throw std::logic_error(
                "Creating IdivInstruction with null operand");
        }
    }
    std::shared_ptr<AssemblyType> getType() { return type; }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in IdivInstruction");
        }
        this->type = newType;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        if (!newOperand) {
            throw std::logic_error("Setting null operand in IdivInstruction");
        }
        this->operand = newOperand;
    }
};

class CdqInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;

  public:
    explicit CdqInstruction(std::shared_ptr<AssemblyType> type) : type(type) {}
    std::shared_ptr<AssemblyType> getType() { return type; }
    void setType(std::shared_ptr<AssemblyType> newType) {
        if (!newType) {
            throw std::logic_error("Setting null type in CdqInstruction");
        }
        this->type = newType;
    }
};

class JmpInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit JmpInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string newLabel) { this->label = newLabel; }
};

class JmpCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::string label;

  public:
    explicit JmpCCInstruction(std::shared_ptr<CondCode> condCode,
                              std::string label)
        : condCode(condCode), label(label) {
        if (!condCode) {
            throw std::logic_error(
                "Creating JmpCCInstruction with null condCode");
        }
    }
    std::shared_ptr<CondCode> getCondCode() { return condCode; }
    std::string getLabel() { return label; }
    void setCondCode(std::shared_ptr<CondCode> newCondCode) {
        if (!newCondCode) {
            throw std::logic_error(
                "Setting null condition code in JmpCCInstruction");
        }
        this->condCode = newCondCode;
    }
    void setLabel(std::string newLabel) { this->label = newLabel; }
};

class SetCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::shared_ptr<Operand> operand;

  public:
    explicit SetCCInstruction(std::shared_ptr<CondCode> condCode,
                              std::shared_ptr<Operand> operand)
        : condCode(condCode), operand(operand) {
        if (!condCode) {
            throw std::logic_error(
                "Creating SetCCInstruction with null condCode");
        }
        if (!operand) {
            throw std::logic_error(
                "Creating SetCCInstruction with null operand");
        }
    }
    std::shared_ptr<CondCode> getCondCode() { return condCode; }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setCondCode(std::shared_ptr<CondCode> newCondCode) {
        if (!newCondCode) {
            throw std::logic_error(
                "Setting null condition code in SetCCInstruction");
        }
        this->condCode = newCondCode;
    }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        if (!newOperand) {
            throw std::logic_error("Setting null operand in SetCCInstruction");
        }
        this->operand = newOperand;
    }
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit LabelInstruction(std::string label) : label(label) {}
    std::string getLabel() { return label; }
    void setLabel(std::string newLabel) { this->label = newLabel; }
};

class PushInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand;

  public:
    explicit PushInstruction(std::shared_ptr<Operand> operand)
        : operand(operand) {
        if (!operand) {
            throw std::logic_error(
                "Creating PushInstruction with null operand");
        }
    }
    std::shared_ptr<Operand> getOperand() { return operand; }
    void setOperand(std::shared_ptr<Operand> newOperand) {
        if (!newOperand) {
            throw std::logic_error("Setting null operand in PushInstruction");
        }
        this->operand = newOperand;
    }
};

class CallInstruction : public Instruction {
  private:
    std::string functionIdentifier;

  public:
    explicit CallInstruction(std::string functionIdentifier)
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
    explicit FunctionDefinition(
        std::string functionIdentifier, bool global,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody,
        size_t stackSize)
        : functionIdentifier(functionIdentifier), global(global),
          functionBody(functionBody), stackSize(stackSize) {
        if (!functionBody) {
            throw std::logic_error(
                "Creating FunctionDefinition with null functionBody");
        }
    }
    std::string getFunctionIdentifier() { return functionIdentifier; }
    bool isGlobal() { return global; }
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
    getFunctionBody() {
        return functionBody;
    }
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        newFunctionBody) {
        if (!newFunctionBody) {
            throw std::logic_error(
                "Setting null function body in FunctionDefinition");
        }
        this->functionBody = newFunctionBody;
    }
    size_t getStackSize() { return stackSize; }
    void setStackSize(size_t newStackSize) { this->stackSize = newStackSize; }
};

class StaticVariable : public TopLevel {
  private:
    std::string identifier;
    bool global;
    int alignment;
    std::shared_ptr<AST::StaticInit> staticInit;

  public:
    explicit StaticVariable(std::string identifier, bool global, int alignment,
                            std::shared_ptr<AST::StaticInit> staticInit)
        : identifier(identifier), global(global), alignment(alignment),
          staticInit(staticInit) {
        if (!staticInit) {
            throw std::logic_error(
                "Creating StaticVariable with null staticInit");
        }
    }
    std::string getIdentifier() { return identifier; }
    bool isGlobal() { return global; }
    int getAlignment() { return alignment; }
    void setAlignment(int newAlignment) { this->alignment = newAlignment; }
    std::shared_ptr<AST::StaticInit> getStaticInit() { return staticInit; }
    void setStaticInit(std::shared_ptr<AST::StaticInit> newStaticInit) {
        if (!newStaticInit) {
            throw std::logic_error(
                "Setting null static init in StaticVariable");
        }
        this->staticInit = newStaticInit;
    }
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels;

  public:
    explicit Program(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels)
        : topLevels(topLevels) {}
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> getTopLevels() {
        return topLevels;
    }
    void setTopLevels(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> newTopLevels) {
        if (!newTopLevels) {
            throw std::logic_error("Setting null top levels in Program");
        }
        this->topLevels = newTopLevels;
    }
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_H
