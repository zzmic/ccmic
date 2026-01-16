#ifndef BACKEND_ASSEMBLY_H
#define BACKEND_ASSEMBLY_H

#include "../frontend/semanticAnalysisPasses.h"
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
    virtual int getImmediate() const;
    virtual std::shared_ptr<Register> getRegister() const;
    virtual std::shared_ptr<ReservedRegister> getReservedRegister() const;
    virtual std::string getPseudoRegister() const;
    virtual int getOffset() const;
    virtual std::string getIdentifier() const;
};

class ImmediateOperand : public Operand {
  private:
    long imm = 0;

  public:
    explicit ImmediateOperand(int imm);
    explicit ImmediateOperand(long imm);
    int getImmediate() const override;
    long getImmediateLong() const;
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
    explicit RegisterOperand(std::shared_ptr<Register> reg);
    explicit RegisterOperand(std::string regInStr);
    std::shared_ptr<Register> getRegister() const override;
    std::string getRegisterInBytesInStr(int size) const;
};

class PseudoRegisterOperand : public Operand {
  private:
    std::string pseudoReg;

  public:
    explicit PseudoRegisterOperand(std::string pseudoReg);
    std::string getPseudoRegister() const override;
};

class StackOperand : public Operand {
  private:
    int offset = 0;
    std::shared_ptr<ReservedRegister> reservedReg;

  public:
    explicit StackOperand(int offset,
                          std::shared_ptr<ReservedRegister> reservedReg);
    int getOffset() const override;
    std::shared_ptr<ReservedRegister> getReservedRegister() const override;
    std::string getReservedRegisterInStr() const;
};

class DataOperand : public Operand {
  private:
    std::string identifier;

  public:
    explicit DataOperand(std::string identifier);
    std::string getIdentifier() const override;
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
                            std::shared_ptr<Operand> dst);
    std::shared_ptr<AssemblyType> getType();
    std::shared_ptr<Operand> getSrc();
    std::shared_ptr<Operand> getDst();
    void setType(std::shared_ptr<AssemblyType> newType);
    void setSrc(std::shared_ptr<Operand> newSrc);
    void setDst(std::shared_ptr<Operand> newDst);
};

class MovsxInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> src, dst;

  public:
    explicit MovsxInstruction(std::shared_ptr<Operand> src,
                              std::shared_ptr<Operand> dst);
    std::shared_ptr<Operand> getSrc();
    std::shared_ptr<Operand> getDst();
    void setSrc(std::shared_ptr<Operand> newSrc);
    void setDst(std::shared_ptr<Operand> newDst);
};

class UnaryInstruction : public Instruction {
  private:
    std::shared_ptr<UnaryOperator> unaryOperator;
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand;

  public:
    explicit UnaryInstruction(std::shared_ptr<UnaryOperator> unaryOperator,
                              std::shared_ptr<AssemblyType> type,
                              std::shared_ptr<Operand> operand);
    std::shared_ptr<UnaryOperator> getUnaryOperator();
    std::shared_ptr<AssemblyType> getType();
    std::shared_ptr<Operand> getOperand();
    void setUnaryOperator(std::shared_ptr<UnaryOperator> newUnaryOperator);
    void setType(std::shared_ptr<AssemblyType> newType);
    void setOperand(std::shared_ptr<Operand> newOperand);
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
                               std::shared_ptr<Operand> operand2);
    std::shared_ptr<BinaryOperator> getBinaryOperator();
    std::shared_ptr<AssemblyType> getType();
    std::shared_ptr<Operand> getOperand1();
    std::shared_ptr<Operand> getOperand2();
    void setBinaryOperator(std::shared_ptr<BinaryOperator> newBinaryOperator);
    void setType(std::shared_ptr<AssemblyType> newType);
    void setOperand1(std::shared_ptr<Operand> newOperand1);
    void setOperand2(std::shared_ptr<Operand> newOperand2);
};

class CmpInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand1, operand2;

  public:
    explicit CmpInstruction(std::shared_ptr<AssemblyType> type,
                            std::shared_ptr<Operand> operand1,
                            std::shared_ptr<Operand> operand2);
    std::shared_ptr<AssemblyType> getType();
    std::shared_ptr<Operand> getOperand1();
    std::shared_ptr<Operand> getOperand2();
    void setType(std::shared_ptr<AssemblyType> newType);
    void setOperand1(std::shared_ptr<Operand> newOperand1);
    void setOperand2(std::shared_ptr<Operand> newOperand2);
};

class IdivInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;
    std::shared_ptr<Operand> operand;

  public:
    explicit IdivInstruction(std::shared_ptr<AssemblyType> type,
                             std::shared_ptr<Operand> operand);
    std::shared_ptr<AssemblyType> getType();
    std::shared_ptr<Operand> getOperand();
    void setType(std::shared_ptr<AssemblyType> newType);
    void setOperand(std::shared_ptr<Operand> newOperand);
};

class CdqInstruction : public Instruction {
  private:
    std::shared_ptr<AssemblyType> type;

  public:
    explicit CdqInstruction(std::shared_ptr<AssemblyType> type);
    std::shared_ptr<AssemblyType> getType();
    void setType(std::shared_ptr<AssemblyType> newType);
};

class JmpInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit JmpInstruction(std::string label);
    std::string getLabel();
    void setLabel(std::string newLabel);
};

class JmpCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::string label;

  public:
    explicit JmpCCInstruction(std::shared_ptr<CondCode> condCode,
                              std::string label);
    std::shared_ptr<CondCode> getCondCode();
    std::string getLabel();
    void setCondCode(std::shared_ptr<CondCode> newCondCode);
    void setLabel(std::string newLabel);
};

class SetCCInstruction : public Instruction {
  private:
    std::shared_ptr<CondCode> condCode;
    std::shared_ptr<Operand> operand;

  public:
    explicit SetCCInstruction(std::shared_ptr<CondCode> condCode,
                              std::shared_ptr<Operand> operand);
    std::shared_ptr<CondCode> getCondCode();
    std::shared_ptr<Operand> getOperand();
    void setCondCode(std::shared_ptr<CondCode> newCondCode);
    void setOperand(std::shared_ptr<Operand> newOperand);
};

class LabelInstruction : public Instruction {
  private:
    std::string label;

  public:
    explicit LabelInstruction(std::string label);
    std::string getLabel();
    void setLabel(std::string newLabel);
};

class PushInstruction : public Instruction {
  private:
    std::shared_ptr<Operand> operand;

  public:
    explicit PushInstruction(std::shared_ptr<Operand> operand);
    std::shared_ptr<Operand> getOperand();
    void setOperand(std::shared_ptr<Operand> newOperand);
};

class CallInstruction : public Instruction {
  private:
    std::string functionIdentifier;

  public:
    explicit CallInstruction(std::string functionIdentifier);
    std::string getFunctionIdentifier();
};

class RetInstruction : public Instruction {};

class TopLevel {
  public:
    virtual ~TopLevel() = default;
};

class FunctionDefinition : public TopLevel {
  private:
    std::string functionIdentifier;
    bool global = false;
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody;
    size_t stackSize = 0;

  public:
    explicit FunctionDefinition(
        std::string functionIdentifier, bool global,
        std::shared_ptr<std::vector<std::shared_ptr<Instruction>>> functionBody,
        size_t stackSize);
    std::string getFunctionIdentifier();
    bool isGlobal();
    std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
    getFunctionBody();
    void
    setFunctionBody(std::shared_ptr<std::vector<std::shared_ptr<Instruction>>>
                        newFunctionBody);
    size_t getStackSize();
    void setStackSize(size_t newStackSize);
};

class StaticVariable : public TopLevel {
  private:
    std::string identifier;
    bool global = false;
    int alignment = 0;
    std::shared_ptr<AST::StaticInit> staticInit;

  public:
    explicit StaticVariable(std::string identifier, bool global, int alignment,
                            std::shared_ptr<AST::StaticInit> staticInit);
    std::string getIdentifier();
    bool isGlobal();
    int getAlignment();
    void setAlignment(int newAlignment);
    std::shared_ptr<AST::StaticInit> getStaticInit();
    void setStaticInit(std::shared_ptr<AST::StaticInit> newStaticInit);
};

class Program {
  private:
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels;

  public:
    explicit Program(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> topLevels);
    std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> getTopLevels();
    void setTopLevels(
        std::shared_ptr<std::vector<std::shared_ptr<TopLevel>>> newTopLevels);
};
} // namespace Assembly

#endif // BACKEND_ASSEMBLY_H
